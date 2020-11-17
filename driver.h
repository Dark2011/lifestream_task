#pragma once


#include "fileprocessor.h"
#include "transport.h"

#include <unordered_map>


class client_driver
{
public:
	enum class mode { siglethread, multithread};

	client_driver(boost::asio::io_context& service, const std::string& ipaddr, int port);	


public:
	void initialaze(const std::filesystem::path& path, const int max_pack_size, const int random_repeat_pack_precentege);
	void run(mode);

private:
	void run_impl(uint64_t key);

private:
	std::unordered_map<uint64_t, file_processor_ptr> _processors;
	std::shared_ptr<transport> _transport;
};