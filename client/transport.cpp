#include "transport.h"

#include <iostream>
#include <boost/bind.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>


using namespace boost::asio;



transport::transport(io_service& service, const std::string& addr, uint16_t port)
	: _service{ service }
	, _ep{ ip::address::from_string(addr), port }
	, _socket{ _service, ip::udp::endpoint(ip::udp::v4(), 0) }
	, _timer{ _service }
{
	;
}


bool transport::send_package(const std::vector<uint8_t>& package, int send_timeout)
{
	std::unique_lock<std::mutex> lock(_operation_access_lock);	
	std::unique_lock<std::mutex> compl_lock(_complete_operation_lock);	
	_done.store(false);	

	_last_send_bytes = 0;
	_timer.expires_from_now(boost::posix_time::milliseconds(send_timeout));
	_timer.async_wait(boost::bind(&transport::on_timer, shared_from_this(), placeholders::error));

	_socket.async_send_to(buffer(package.data(), package.size()), _ep,
		boost::bind(&transport::on_send, shared_from_this(), placeholders::error, placeholders::bytes_transferred));

	while(!_done.load()) //spurious wake up protect
		_complete_condition.wait_for(compl_lock, std::chrono::milliseconds(10));

	return _last_send_bytes;
}


bool transport::recv_package(int recv_timeout)
{
	std::unique_lock<std::mutex> lock(_operation_access_lock);	
	std::unique_lock<std::mutex> compl_lock(_complete_operation_lock);	
	_done.store(false);		
	

	_last_read_bytes = 0;
	_timer.expires_from_now(boost::posix_time::milliseconds(recv_timeout));
	_timer.async_wait(boost::bind(&transport::on_timer, shared_from_this(), boost::asio::placeholders::error));

	ip::udp::endpoint server_ep;

	_socket.async_receive_from(buffer(_receive_data, max_length), server_ep,
		boost::bind(&transport::on_recv, shared_from_this(), placeholders::error, placeholders::bytes_transferred));
	
	while(!_done.load())
		_complete_condition.wait_for(compl_lock, std::chrono::milliseconds(10));

	return _last_read_bytes;
}


void transport::on_send(const boost::system::error_code& ec, size_t size)
{
	if (!ec && size > 0)
	{
		std::unique_lock<std::mutex> lock(_timer_lock);	
		_timer.cancel();
		BOOST_LOG_TRIVIAL(trace) << size << " bytes package has been sended";
		//std::cout << size << " bytes package has been sended" << std::endl;

		_last_send_bytes = size;	
		_last_error = ec;
		_done.store(true);
		_complete_condition.notify_all();
	}
	else
	{
		if (boost::asio::error::operation_aborted == ec.value())
		{
			_last_send_bytes = 0;
			// it's not an error case - just cancel after successeful on_tiner abort operations
			BOOST_LOG_TRIVIAL(trace) << "Send operation aborted by timer (transport)";
			//std::cout << "Receive operation aborted (transport)" << std::endl;
			return;
		}
	}
}


void transport::on_recv(const boost::system::error_code& ec, size_t size)
{
	if (!ec && size > 0)
	{
		std::unique_lock<std::mutex> lock(_timer_lock);	
		BOOST_LOG_TRIVIAL(trace) << size << " bytes package has been received";
		//std::cout << size << " bytes package has been received" << std::endl;

		_last_read_bytes = size;
		_timer.cancel();
		_last_error = ec;
		_done.store(true);
		_complete_condition.notify_all();
	}
	else
	{
		if (boost::asio::error::operation_aborted == ec.value())
		{
			_last_read_bytes = 0;
			// it's not an error case - just cancel after successeful on_tiner abort operations
			BOOST_LOG_TRIVIAL(trace) << "Receive operation aborted by timer (transport)";
			//std::cout << "Receive operation aborted (transport)" << std::endl;
			return;
		}


		_done.store(false);
		ip::udp::endpoint server_ep;
		_socket.async_receive_from(buffer(_receive_data, max_length), server_ep,
			boost::bind(&transport::on_recv, shared_from_this(), placeholders::error, placeholders::bytes_transferred));
	}
}



void transport::on_timer(const boost::system::error_code& ec)
{
	_last_error = ec;
	if (ec)
	{
		if (boost::asio::error::operation_aborted == ec.value())
		{
			// it's not an error case - just cancel after successeful on_send or on_recv operations
			BOOST_LOG_TRIVIAL(trace) << "Timer operation aborted (transport)";
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
	_done.store(true);
	_complete_condition.notify_all();
}



const boost::system::error_code& transport::get_last_error() const { return _last_error; }



std::vector<uint8_t> transport::get_recv_data()
{
	std::unique_lock<std::mutex> lock(_operation_access_lock);
	std::vector<uint8_t> result(_last_read_bytes);
	std::copy(_receive_data, _receive_data + _last_read_bytes, result.begin());
	return result;
}
