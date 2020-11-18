#pragma once


#include "fileprocessor.h"
#include "transport.h"

#include <unordered_map>
#include <queue>




class client_driver
{
public:
	enum class mode { singlethread, multithread};

	client_driver(boost::asio::io_context& service, const std::string& ipaddr, int port);	
	~client_driver();

public:
	void initialaze(const std::experimental::filesystem::path& path, const int max_pack_size, const int random_repeat_pack_precentege);
	void run(mode);

private:
	void worker_thread();
	void run_impl_single(uint64_t key);
	void run_impl_multi(uint64_t key);

private:
	std::unordered_map<uint64_t, file_processor_ptr> _processors;
	std::shared_ptr<transport> _transport;
	//
	std::atomic_bool						  _done;
	std::mutex								  _task_lock;
	std::queue<std::function<void()>>		  _tasks;
	std::vector<std::thread>				  _thread_pool;
};