#include "driver.h"



ServerDriver::ServerDriver(int port, QObject* parent)
	: QObject(parent)
	, _server{ std::make_unique<SimpleUdpServer>(port, parent) }
{
	connect(_server.get(), &SimpleUdpServer::processTheDatagram, this, &ServerDriver::onMessage);
}


void ServerDriver::onMessage(const QNetworkDatagram& package)
{
	QByteArray datagram = package.data();
	if (datagram.size() >= HEADER_SIZE)
	{
		message_hdr* hdr = (message_hdr*)(datagram.data());
		uint64_t id = *(uint64_t*)hdr->_id;

		if (!_processors.count(id))
		{
			_processors.insert(std::make_pair(id, std::make_shared<FileProcessor>(id)));
			std::cout << "Processor: " << id << " has been created" << std::endl;
		}

		_processors[id]->handleMessage(datagram);
		_server->sendAck(_processors[id]->createAckPackage());
		std::cout << "=========================" << std::endl;

		if (_processors[id]->isFinished())
		{
			std::cout << "Processor: " << id << " is done" << std::endl;
			_processors.erase(id);
		}
	}
}
