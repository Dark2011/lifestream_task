#include "fileprocessor.h"

#include <QFile>
#include <iostream>



FileProcessor::FileProcessor(uint64_t id)
	: _id{ id }
	, _state{ state::Begin }
{
	;
}


void FileProcessor::handleMessage(const QByteArray& datagram)
{
	int size = datagram.size();
	if (size >= HEADER_SIZE)
	{
		message_hdr* hdr = (message_hdr*)(datagram.data());
		if (_state == state::Begin)
		{
			_totalPacks = hdr->_seqTotal;
			_receivedData.resize(MAX_PAYLOAD * _totalPacks);
			_state = state::InProgress;
		}

		if (_state == state::InProgress)
		{
			_lastPack = hdr->_seqNumber;
			if (!_receivedPackNums.count(_lastPack))
			{
				_receivedPackNums.insert(_lastPack);
				++_receivedPacks;

				size_t payload = datagram.size() - HEADER_SIZE;

				memcpy((void*)&_receivedData[_lastPack * MAX_PAYLOAD], (uint8_t*)(datagram.data() + HEADER_SIZE), payload);
				_totalSize += payload;

				std::cout << "FP id: " << _id << " receive PUT package, seqNumber: " << _lastPack << " receivedPacks: " << _receivedPacks << std::endl;

				if (_receivedPacks == _totalPacks)
				{
					_state = state::End;
					_receivedData.resize(_totalSize);
				}
			}
			else
			{
				std::cout << "!!! Package " << _lastPack << " has already been received (Idempotent condition) !!!" << std::endl;
			}
		}
	}
}


bool FileProcessor::isFinished() const 
{
	if (_state == state::End)
	{
#ifdef SAVE_TO_FS
		QFile file(QString("download/download_%1").arg(_id));
		if (file.open(QIODevice::WriteOnly))
		{
			file.write((char*)&_receivedData[0], _receivedData.size());
			file.close();
		}
#endif		
		return true;
	}
	return false;
}


uint64_t			 FileProcessor::getId()    const { return _id; }
FileProcessor::state FileProcessor::getState() const { return _state; }


QByteArray FileProcessor::createAckPackage() const
{
	QByteArray result;
	message_hdr hdr;
	memcpy(hdr._id, &_id, sizeof(hdr._id));
	hdr._seqNumber = _lastPack;
	hdr._seqTotal = _receivedPacks;
	hdr._type = ACK;

	size_t payload = _state == state::End ? HEADER_SIZE + sizeof(uint32_t) : HEADER_SIZE;

	std::cout << "FP id: " << _id << " send ACK package, seqNumber: " << _lastPack << " receivedPacks: " << _receivedPacks << std::endl;
	result.resize(payload);
	memcpy(result.data(), &hdr, HEADER_SIZE);

	if (state::End == _state)
	{		
		uint32_t crc = crc32c(0, _receivedData.data(), _receivedData.size());
		memcpy(result.data() + HEADER_SIZE, &crc, sizeof(uint32_t));
		std::cout << "Last package has been received, send ACK package with CRC: " << crc << std::endl;
	}
	//
	return result;
}
