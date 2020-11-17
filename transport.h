#pragma once

#include <vector>
#include <mutex>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/system/error_code.hpp>


#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>


using namespace boost::asio;


class transport : public std::enable_shared_from_this<transport>, private boost::noncopyable
{
	using receive_callback = std::function<void(uint64_t, std::vector<uint8_t>)>;

public:
	transport(io_service& service, const std::string& addr, int port)
		: _service { service }
		, _ep { ip::address::from_string(addr), port }
		, _socket {_service, ip::udp::endpoint(ip::udp::v4(), 0)}
		, _timer{_service }
	{
		;
	}

	bool send_package(const std::vector<uint8_t>& package, int send_timeout = 30 /*ms*/)
	{	
		std::lock_guard<std::mutex> lock(_operation_access_lock);
		std::unique_lock<std::mutex> compl_lock(_complete_operation_lock);

		_timer.expires_from_now(boost::posix_time::milliseconds(send_timeout));
		_timer.async_wait(boost::bind(&transport::on_timer, shared_from_this(), placeholders::error));

		_socket.async_send_to(buffer(package.data(), package.size()), _ep, 
			boost::bind(&transport::on_send, shared_from_this(), placeholders::error, placeholders::bytes_transferred));

		_complete_condition.wait(compl_lock);

		return _last_send_bytes;
	}


	bool recv_package(std::vector<uint8_t>& recv_data, int recv_timeout = 300 /*ms*/)
	{
		std::lock_guard<std::mutex> lock(_operation_access_lock);
		std::unique_lock<std::mutex> compl_lock(_complete_operation_lock);

		//std::cout << "Do async receive operation" << std::endl;

		_last_read_bytes = 0;
		_timer.expires_from_now(boost::posix_time::milliseconds(recv_timeout));
		_timer.async_wait(boost::bind(&transport::on_timer, shared_from_this(), boost::asio::placeholders::error));

		ip::udp::endpoint server_ep;		
			
		_socket.async_receive_from(buffer(_receive_data, max_length), server_ep,
			boost::bind(&transport::on_recv, shared_from_this(), placeholders::error, placeholders::bytes_transferred));
		_complete_condition.wait(compl_lock);
		//std::cout << "Finish async receive operation" << std::endl;
		recv_data.clear();
		recv_data.resize(_last_read_bytes);
		std::copy(_receive_data, _receive_data + _last_read_bytes, recv_data.begin());
		return _last_read_bytes;
	}


	void on_send(const boost::system::error_code& ec, size_t size)
	{
		std::unique_lock<std::mutex> lock(_timer_lock);
		BOOST_LOG_TRIVIAL(trace) << size << " bytes package has been sended";
		//std::cout << size << " bytes package has been sended" << std::endl;
		_timer.cancel();
		_last_error = ec;
		_last_send_bytes = size;
		_complete_condition.notify_all();
	}



	void on_recv(const boost::system::error_code& ec, size_t size)
	{
		std::unique_lock<std::mutex> lock(_timer_lock);
		//BOOST_LOG_TRIVIAL(trace) << size << " bytes package has been received";
		//std::cout << size << " bytes package has been received" << std::endl;
		_last_error = ec;
		if (!ec && size > 0)
		{
			BOOST_LOG_TRIVIAL(trace) << size << " bytes package has been received";
			//std::cout << "Successefull async receive operation" << std::endl;
			_last_read_bytes = size;
			_timer.cancel();
			_last_error = ec;
			_complete_condition.notify_all();
		}
		else
		{
			//std::cout << "Repeat async receive operation" << std::endl;
			ip::udp::endpoint server_ep;
			_socket.async_receive_from(buffer(_receive_data, max_length), server_ep,
				boost::bind(&transport::on_recv, shared_from_this(), placeholders::error, placeholders::bytes_transferred));
		}		
	}


	void on_timer(const boost::system::error_code& ec)
	{		
		_last_error = ec;
		if (ec)
		{			
			if (boost::asio::error::operation_aborted == ec.value()) 
			{
				// it's not an error case - just cancel after successeful on_send or on_recv operations
				//std::cout << "Timer operation aborted (transport)" << std::endl;
				return;
			}
			else
			{
				BOOST_LOG_TRIVIAL(error) << "Timer operation error (transport)";
				//std::cerr << "Timer operation error (transport)" << std::endl;
			}
		}
		else    BOOST_LOG_TRIVIAL(warning) << "Timer operation timeout (transport)";
			//std::cerr << "Timer operation timeout (transport)" << std::endl;

		std::unique_lock<std::mutex> lock(_timer_lock);
		memset(_receive_data, 0, sizeof(_receive_data));
		_socket.cancel();
		_complete_condition.notify_all();
	}


	const boost::system::error_code& get_last_error() const { return _last_error; }


	//std::vector<uint8_t> get_recv_data()  
	//{
	//	std::lock_guard<std::mutex> lock(_operation_access_lock);
	//	std::vector<uint8_t> result(_last_read_bytes);
	//	std::copy(_receive_data, _receive_data + _last_read_bytes, result.begin());
	//	return result;
	//}


private:
	enum { max_length = sizeof(message_hdr) + sizeof(uint32_t) };
	uint8_t _receive_data[max_length];
	size_t _last_read_bytes = 0;
	size_t _last_send_bytes = 0;

	io_service&			_service;
	ip::udp::endpoint	_ep;
	ip::udp::socket		_socket;

	deadline_timer		_timer;

	std::mutex _operation_access_lock;
	std::mutex _complete_operation_lock;
	std::mutex _timer_lock;
	std::condition_variable _complete_condition;

	boost::system::error_code _last_error;
};

