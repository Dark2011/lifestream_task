#pragma once

#include "message.h"

#include <QByteArray>

#include <vector>
#include <memory>
#include <set>


class FileProcessor 
{

public:
	enum class state
	{
		Begin,
		InProgress,
		End
	};

	FileProcessor(uint64_t id);
	//
	void handleMessage(const QByteArray& datagram);	
	//
	bool		isFinished() const;
	uint64_t	getId()		 const; 
	state		getState()	 const;
	//
	QByteArray createAckPackage() const;

private:
	uint64_t _id;
	std::vector<uint8_t> _receivedData;
	//
	state _state;
	//
	uint32_t _lastPack = 0;
	uint32_t _receivedPacks = 0;
	//
	uint32_t _totalPacks = 0;
	uint64_t _totalSize = 0;
	std::set<uint32_t> _receivedPackNums;

};

using shared_file_processor = std::shared_ptr<FileProcessor>;
