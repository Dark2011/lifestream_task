#include "server.h"


#pragma once

#include <QObject>
#include <QtNetwork>

#include <iostream>
#include "message.h"




SimpleUdpServer::SimpleUdpServer(int port, QObject* parent)
	: QObject{ parent }
	, _sock (std::make_unique<QUdpSocket>(this))
{
	_sock->bind(QHostAddress::LocalHost, port);
	connect(_sock.get(), &QUdpSocket::readyRead, this, &SimpleUdpServer::onPending);
}


/*public slots*/
void SimpleUdpServer::onPending()
{
	while (_sock->hasPendingDatagrams())
	{
		QNetworkDatagram datagram = _sock->receiveDatagram();
		_sender = datagram.senderAddress();
		_senderPort = datagram.senderPort();
		emit processTheDatagram(datagram);
	}
}


void SimpleUdpServer::sendAck(const QByteArray& data)
{
	_sock->writeDatagram(data, _sender, _senderPort);
}

