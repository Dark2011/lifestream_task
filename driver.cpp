#include "driver.h"

#include <boost/system/error_code.hpp>


#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>


client_driver::client_driver(boost::asio::io_context& service, const std::string& ipaddr, int port)
		: _transport{ new transport(service, ipaddr, port) }
		, _processors{}
	{
		;
	}



void client_driver::initialaze(const std::filesystem::path& path, const int max_pack_size, const int random_repeat_pack_precentege)
{
	for (auto& file : std::filesystem::directory_iterator(path))
	{
		if (std::filesystem::is_regular_file(file.symlink_status()))
		{
			file_processor_ptr tf{ std::make_unique<file_processor>(file) };
			if (tf->initialize(max_pack_size, random_repeat_pack_precentege))
				_processors.insert(std::pair(tf->get_id(), std::move(tf)));
		}
	}
}


void client_driver::run(mode mode)
{
	//std::vector<std::thread> handlers;
	//for (auto& pr : _processors)
	//	handlers.emplace_back(&client_driver::run_impl, this, pr.first);

	//std::for_each(handlers.begin(), handlers.end(), [](auto& thread)
	//	{
	//		thread.join();
	//		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	//	});

	if (mode == mode::siglethread)
	{
		uint64_t finished = 0;
		while (_processors.size())
		{
			for (auto& iter = _processors.begin(); iter != _processors.end(); ++iter)
			{
				run_impl(iter->first);
				if (iter->second->get_state() == file_processor::state::Done)
					finished = iter->first;
			}
			_processors.erase(finished);
		}
	}
	else
	{
		std::vector<std::thread> handlers;
		for (auto& pr : _processors)
			handlers.emplace_back(&client_driver::run_impl, this, pr.first);

		std::for_each(handlers.begin(), handlers.end(), [](auto& thread)
		{
			thread.join();
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		});
	}


	std::cout << "Done!!!" << std::endl;
}


void client_driver::run_impl(uint64_t key)
{
	auto& proc = _processors[key];
	while (file_processor::state::Done != proc->get_state())
	//if(file_processor::state::Done != proc->get_state())
	{
		//std::cout << "handle file processor " << key << std::endl;
		BOOST_LOG_TRIVIAL(trace) << "handle file processor " << key;
		
		std::vector<uint8_t> recv_data;
		auto package = proc->get_next_package();
		if (_transport->send_package(package))
		{
			if (_transport->recv_package(recv_data))
			{
				//auto recv_buffer = _transport->get_recv_data();
				if (recv_data.size() >= sizeof(message_hdr))
				{
					message_hdr* hdr = (message_hdr*)&recv_data[0];
					proc->handle_ack_package(hdr);
				}
			}

		}
		BOOST_LOG_TRIVIAL(trace) << "=========================";
		//std::cout << "=========================" << std::endl;
	}
	//else
	//{
	//	BOOST_LOG_TRIVIAL(trace) << "Processor " << key << " is done";
	//	//std::cout << "Processor " << key << " is done" << std::endl;
	//	//_processors.erase(key);
	//}

	BOOST_LOG_TRIVIAL(trace) << "Processor " << key << " is done";
	//_processors.erase(key);
}


