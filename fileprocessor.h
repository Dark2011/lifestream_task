#pragma once


#include "message.h"


#include <string>
#include <vector>
//#include <filesystem>
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
#include <set>

#include <boost/noncopyable.hpp>



class file_processor : private boost::noncopyable
{
public:
	enum class state { Init, InProgress, Done };

	file_processor(const std::experimental::filesystem::path& path = std::experimental::filesystem::current_path());
	//
	bool initialize(size_t max_pack_size, uint8_t repeat_pack_perc);
	//
	const std::vector<uint8_t>& get_next_package() const;
	void handle_ack_package(const std::vector<uint8_t>& recv_data);
	//
	state get_state() const;
	uint64_t get_id() const;	

private:
	size_t get_current_datapack_size(uint32_t i) const;

private:
	std::experimental::filesystem::path _path;
	//
	state				 _state;
	size_t				 _max_pack_size = 0;
	//
	uint8_t				 _id[ID_LENGTH];
	std::vector<uint8_t> _data;	
	std::vector<int>	 _jobs;
	uint32_t			 _iter = 0;
	//
	mutable std::vector<uint8_t> _current_package;
	uint32_t					 _packages_cnt = 0;
	uint32_t					 _remaining_data = 0;	
	//
	mutable std::set<uint32_t>	 _send_packet_checker;
};


using file_processor_ptr = std::unique_ptr<file_processor>;
