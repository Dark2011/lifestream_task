#pragma once


#include "message.h"

#include <mutex>

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/system/error_code.hpp>
#include <boost/thread.hpp>



class transport : public std::enable_shared_from_this<transport>, private boost::noncopyable
{
public:
	transport(boost::asio::io_service& service, const std::string& addr, uint16_t port);
	//
	bool send_package(const std::vector<uint8_t>& package, int send_timeout = 10 /*ms*/);
	bool recv_package(int recv_timeout = 50 /*ms*/);
	//
	void on_send(const boost::system::error_code& ec, size_t size);
	void on_recv(const boost::system::error_code& ec, size_t size);
	void on_timer(const boost::system::error_code& ec);
	//
	const boost::system::error_code& get_last_error() const;
	std::vector<uint8_t> get_recv_data();

private:
	enum { max_length = HEADER_SIZE + sizeof(uint32_t) };
	uint8_t _receive_data[max_length];
	size_t _last_read_bytes = 0;
	size_t _last_send_bytes = 0;
	//
	boost::asio::io_service&		_service;
	boost::asio::ip::udp::endpoint		_ep;
	boost::asio::ip::udp::socket		_socket;
	boost::asio::deadline_timer		_timer;
	//
	std::atomic_bool			_done{false};
	std::mutex 				_operation_access_lock;
	std::mutex 				_complete_operation_lock;
	std::mutex 				_timer_lock;
	std::condition_variable  		_complete_condition;
	//
	boost::system::error_code 		_last_error;	
};

