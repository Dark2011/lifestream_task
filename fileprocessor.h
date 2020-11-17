#pragma once


#include "message.h"


#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>
#include <set>

#include <boost/noncopyable.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>



class file_processor : private boost::noncopyable
{
public:
	enum class state { Init, InProgress, Done };


	file_processor(const std::filesystem::path& path = std::filesystem::current_path())
		: _path{path}
	{
		;
	}


	std::vector<uint8_t> get_next_package() const 
	{	
		std::vector<uint8_t> dataToSend;
		if (_state == state::Init || _state == state::InProgress)
		{
			message_hdr hdr;

			hdr._seqNumber = _jobs[_iter];
			hdr._seqTotal = _packagesCnt;
			hdr._type = PUT;
			memcpy(hdr._id, _id, sizeof(hdr._id));

			//if (!_send_packet_checker.count(hdr._seqNumber))
			//	_send_packet_checker.insert(hdr._seqNumber);
			//else
			//	std::cout << "packet " << hdr._seqNumber << " has already been sended" << std::endl;
			//BOOST_LOG_TRIVIAL(warning) << "packet " << hdr._seqNumber << " has already been sended";

			size_t payload = get_current_datapack_size(_iter);
			dataToSend.resize(sizeof(message_hdr) + payload);

			//std::copy((uint8_t*)&hdr, (uint8_t*)&hdr + sizeof(message_hdr), dataToSend.begin());
			//int startPos = _jobs[_iter] * _max_pack_size;
			//int endPos = startPos + payload;
			//std::copy(_data.begin() + startPos, _data.begin() + endPos, dataToSend.begin() + sizeof(message_hdr));

			memcpy(&dataToSend[0], &hdr, sizeof(message_hdr));
			memcpy(&dataToSend[sizeof(message_hdr)], &_data[hdr._seqNumber * _max_pack_size], payload);
			//
			//std::cout << "prepair for sending package " << hdr._seqNumber << std::endl;
			BOOST_LOG_TRIVIAL(trace) << "prepair for sending package " << hdr._seqNumber;
		}
		
		return dataToSend;
	}


	void handle_ack_package(message_hdr* hdr)
	{
		//std::cout << "receive ACK package seqNumber: " << hdr->_seqNumber << " seqTotal: " << hdr->_seqTotal << std::endl;
		BOOST_LOG_TRIVIAL(trace) << "receive ACK package seqNumber: " << hdr->_seqNumber << " seqTotal: " << hdr->_seqTotal;

		if (ACK == hdr->_type)
		{
			// here we check whether the header mathces the file processor 
			if (memcmp(hdr->_id, _id, sizeof(hdr->_id)) != 0)
				return;

			if (hdr->_seqNumber == _jobs[_iter] && hdr->_seqTotal != _packagesCnt)
			{
				_state = state::InProgress;
				++_iter;
			}
			else if (hdr->_seqTotal == _packagesCnt)
			{
				//std::cout << "Done " << std::endl;
				BOOST_LOG_TRIVIAL(trace) << "Done ";
				_state = state::Done;
			}
		}
	}


	state get_state() const { return _state; }
	uint64_t get_id() const { return *(uint64_t*)_id; }


	bool initialize(size_t max_pack_size, uint8_t repeat_pack_perc)
	{
		try
		{
			_max_pack_size = max_pack_size - sizeof(message_hdr);

			std::ifstream file(_path, std::ios::binary);
			auto fsize = std::filesystem::file_size(_path);

			_data = std::vector<uint8_t>(fsize);
			file.read((char*)&_data[0], fsize);
			file.close();

			const size_t cnt = fsize / _max_pack_size;
			_remainingData = fsize % _max_pack_size; // last package payload

			for (; _packagesCnt < cnt; ++_packagesCnt)
			{
				_jobs.push_back(_packagesCnt);
				// add some dublicate package to show idempotent properties (same package can be send and receive more than one time)
				if (!(_packagesCnt % repeat_pack_perc))
					_jobs.push_back(_packagesCnt);
			}

			// here we need one additional package for sending _remainingData payload
			if (_remainingData)
			{
				_jobs.push_back(_packagesCnt);
				++_packagesCnt;			
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
		catch(...)
		{
			return false;
		}
	}

private:
	size_t get_current_datapack_size(uint32_t i) const
	{
		return _jobs[i] != _packagesCnt-1 ? _max_pack_size : _remainingData;
	}

private:
	std::filesystem::path _path;
	//
	state				 _state;
	size_t				 _max_pack_size = 0;
	//
	uint8_t				 _id[ID_LENGTH];
	std::vector<uint8_t> _data;	
	std::vector<int>	 _jobs;
	//
	uint32_t			 _packagesCnt = 0;
	uint32_t			 _remainingData = 0;
	//
	uint32_t			 _iter = 0;
	//
	mutable std::set<uint32_t>	 _send_packet_checker;
};


using file_processor_ptr = std::unique_ptr<file_processor>;