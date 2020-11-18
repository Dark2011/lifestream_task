#include "driver.h"



#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>



client_driver::client_driver(boost::asio::io_context& service, const std::string& ipaddr, uint16_t port)
	: _processors {}
	, _transport{ new transport(service, ipaddr, port) }
	, _done{false}
{
	;
}


client_driver::~client_driver() { _done = true; }


void client_driver::initialaze(const boost::filesystem::path& path, int max_pack_size, int random_repeat_pack_precentege)
{
	for (auto& file : boost::filesystem::directory_iterator(path))
	{
		if (boost::filesystem::is_regular_file(file.symlink_status()))
		{
			file_processor_ptr tf{ std::make_unique<file_processor>(file) };
			if (tf->initialize(max_pack_size, random_repeat_pack_precentege))
				_processors.insert(std::make_pair(tf->get_id(), std::move(tf)));
		}
	}
}


void client_driver::run(mode mode)
{
	if (client_driver::mode::singlethread == mode)
	{
		uint64_t finishedId = 0;
		while (_processors.size())
		{
			for (auto&& iter = _processors.begin(); iter != _processors.end(); ++iter)
			{
				run_impl_single(iter->first);
				if (iter->second->get_state() == file_processor::state::Done)
					finishedId = iter->first;
			}
			_processors.erase(finishedId);
		}
	}
	else
	{
		for (auto& pr : _processors)
			_tasks.push(std::bind(&client_driver::run_impl_multi, this, pr.first));

		const auto real_cpu = std::thread::hardware_concurrency() / 2;
		for (unsigned int i = 0; i < real_cpu; ++i)
			_thread_pool.push_back(std::thread(&client_driver::worker_thread, this));

		for (unsigned int i = 0; i < _thread_pool.size(); ++i)
			_thread_pool[i].join();
	}

	BOOST_LOG_TRIVIAL(trace) << "All tasks are performed!!!";
	//std::cout << "All tasks are performed!!!" << std::endl;
}


void client_driver::worker_thread()
{
	while (!_done)
	{
		_task_lock.lock();
		if (!_tasks.empty())
		{
			auto task = _tasks.front();
			_tasks.pop();
			_task_lock.unlock();
			task();
		}
		else
		{
			_task_lock.unlock();
			std::this_thread::yield();
			if (_processors.empty())
				_done = true;
		}

	}
}


void client_driver::run_impl_single(uint64_t key)
{
	auto& proc = _processors[key];

	if(file_processor::state::Done != proc->get_state())
	{
		BOOST_LOG_TRIVIAL(trace) << "handle file processor " << key;
		//std::cout << "handle file processor " << key << std::endl;

		auto package = proc->get_next_package();
		if (_transport->send_package(package))
		{
			if (_transport->recv_package())
			{
				auto recv_data = _transport->get_recv_data();
				if (recv_data.size() >= HEADER_SIZE)
					proc->handle_ack_package(recv_data);
			}

		}
		BOOST_LOG_TRIVIAL(trace) << "=========================";
		//std::cout << "=========================" << std::endl;
	}
	else
	{
		BOOST_LOG_TRIVIAL(trace) << "Processor " << key << " is done";
		//std::cout << "Processor " << key << " is done" << std::endl;
	}
}


void client_driver::run_impl_multi(uint64_t key)
{
	auto& proc = _processors[key];
	while (file_processor::state::Done != proc->get_state())
	{		
		BOOST_LOG_TRIVIAL(trace) << "handle file processor " << key;
		//std::cout << "handle file processor " << key << std::endl;
		
		auto package = proc->get_next_package();
		if (_transport->send_package(package))
		{
			if (_transport->recv_package())
			{
				auto recv_data = _transport->get_recv_data();
				if (recv_data.size() >= HEADER_SIZE)
					proc->handle_ack_package(recv_data);
			}

		}
		BOOST_LOG_TRIVIAL(trace) << "=========================";
		//std::cout << "=========================" << std::endl;
	}

	BOOST_LOG_TRIVIAL(trace) << "Processor " << key << " is done";
	_processors.erase(key);
}


