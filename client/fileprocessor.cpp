#include "fileprocessor.h"

#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>



file_processor::file_processor(const boost::filesystem::path& path)
	: _path{ path }
{
	;
}


bool file_processor::initialize(size_t max_pack_size, uint8_t repeat_pack_perc)
{
	try
	{
		_max_pack_size = max_pack_size - HEADER_SIZE;

		std::ifstream file(_path.string(), std::ios::binary);
		auto fsize = boost::filesystem::file_size(_path);

		_data = std::vector<uint8_t>(fsize);
		file.read((char*)&_data[0], fsize);
		file.close();

		const size_t cnt = fsize / _max_pack_size;
		_remaining_data = fsize % _max_pack_size; // last package payload

		for (; _packages_cnt < cnt; ++_packages_cnt)
		{
			_jobs.push_back(_packages_cnt);
			// add some dublicate package to show idempotent properties (same package can be send and receive more than one time)
			if (!(_packages_cnt % repeat_pack_perc))
				_jobs.push_back(_packages_cnt);
		}

		// here we need one additional package for sending _remaining_data payload
		if (_remaining_data)
		{
			_jobs.push_back(_packages_cnt);
			++_packages_cnt;
		}

		std::random_device rd;
		std::mt19937 g(rd());

		memset(&_id, 0, sizeof(_id));
		int r = g();
		memcpy(_id, (void*)&r, sizeof(int));

		// random shuffle packages order 
		std::shuffle(_jobs.begin(), _jobs.end(), g);

		_state = state::Init;
		return true;
	}
	catch (...)
	{
		return false;
	}
}


const std::vector<uint8_t>& file_processor::get_next_package() const
{
	if (_state == state::Init || _state == state::InProgress)
	{
		message_hdr hdr;

		hdr._seqNumber = _jobs[_iter];
		hdr._seqTotal = _packages_cnt;
		hdr._type = PUT;
		memcpy(hdr._id, _id, sizeof(hdr._id));

		if (!_send_packet_checker.count(hdr._seqNumber))
			_send_packet_checker.insert(hdr._seqNumber);
		else			
			BOOST_LOG_TRIVIAL(warning) << "!!! package " << hdr._seqNumber << " has already been sended (Idempotent condition) !!!";
			//std::cout << "!!! package " << hdr._seqNumber << " has already been sended (Idempotent condition) !!!" << std::endl;

		const size_t payload = get_current_datapack_size(_iter);
		if(HEADER_SIZE + payload != _current_package.size())
			_current_package.resize(HEADER_SIZE + payload);

		memcpy(&_current_package[0], &hdr, HEADER_SIZE);
		memcpy(&_current_package[HEADER_SIZE], &_data[hdr._seqNumber * _max_pack_size], payload);
		//		
		BOOST_LOG_TRIVIAL(trace) << "prepair for sending package " << hdr._seqNumber;
		//std::cout << "prepair for sending package " << hdr._seqNumber << std::endl;
	}

	return _current_package;
}


void file_processor::handle_ack_package(const std::vector<uint8_t>& recv_data)
{	
	message_hdr hdr = *(message_hdr*)&recv_data[0];
	BOOST_LOG_TRIVIAL(trace) << "receive ACK package seqNumber: " << hdr._seqNumber << " seqTotal: " << hdr._seqTotal;
	//std::cout << "receive ACK package seqNumber: " << hdr._seqNumber << " seqTotal: " << hdr._seqTotal << std::endl;

	if (ACK == hdr._type)
	{
		// here we check whether the header mathces the file processor 
		if (memcmp(hdr._id, _id, sizeof(hdr._id)) != 0)
			return;

		if (hdr._seqNumber == _jobs[_iter] && hdr._seqTotal != _packages_cnt)
		{
			_state = state::InProgress;
			++_iter;
		}
		else if (hdr._seqTotal == _packages_cnt)
		{
			uint32_t server_crc = 0; 
			memcpy(&server_crc, recv_data.data() + HEADER_SIZE, sizeof(uint32_t));

			uint32_t client_crc = crc32c(0, _data.data(), _data.size());

			BOOST_LOG_TRIVIAL(trace) << "!!! Done, server crc: " << server_crc << ", client crc: " << client_crc << " !!!";
			if (server_crc == client_crc)
				 BOOST_LOG_TRIVIAL(trace) << "NO ERROR, CRC is the same";
			else BOOST_LOG_TRIVIAL(trace) << "THERE IS AN ERROR in CRC, not the same";			

			_state = state::Done;
		}
	}
}


file_processor::state file_processor::get_state() const { return _state; }


uint64_t file_processor::get_id() const { return *(uint64_t*)_id; }


size_t file_processor::get_current_datapack_size(uint32_t i) const
{
	return _jobs[i] != _packages_cnt - 1 ? _max_pack_size : _remaining_data;
}

