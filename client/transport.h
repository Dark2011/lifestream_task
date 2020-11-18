#pragma once


#include "message.h"

#include <vector>
#include <mutex>

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/system/error_code.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>



class transport : public std::enable_shared_from_this<transport>, private boost::noncopyable
{
public:
	transport(boost::asio::io_service& service, const std::string& addr, int port);
	//
	bool send_package(const std::vector<uint8_t>& package, int send_timeout = 15 /*ms*/);
	bool recv_package(int recv_timeout = 300 /*ms*/);
	//
	void on_send(const boost::system::error_code& ec, size_t size);
	void on_recv(const boost::system::error_code& ec, size_t size);
	void on_timer(const boost::system::error_code& ec);
	//
	const boost::system::error_code& get_last_error() const;
	std::vector<uint8_t> get_recv_data();

private:
	enum { max_length = sizeof(message_hdr) + sizeof(uint32_t) };
	uint8_t _receive_data[max_length];
	size_t _last_read_bytes = 0;
	size_t _last_send_bytes = 0;
	//
	boost::asio::io_service&			_service;
	boost::asio::ip::udp::endpoint		_ep;
	boost::asio::ip::udp::socket		_socket;
	boost::asio::deadline_timer			_timer;
	//
	std::mutex _operation_access_lock;
	std::mutex _complete_operation_lock;
	std::mutex _timer_lock;
	std::mutex _test_lock;
	std::condition_variable _complete_condition;
	//
	boost::system::error_code _last_error;
};

