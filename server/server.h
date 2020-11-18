#pragma once

#include <QObject>
#include <QtNetwork>
#include <QDebug>
#include <iostream>
#include <memory>

#include "message.h"



class SimpleUdpServer : public QObject
{
	Q_OBJECT

public:
	SimpleUdpServer(int port, QObject* parent = nullptr);	

signals:
	void processTheDatagram(const QNetworkDatagram&);

public slots:
	void onPending();
	void sendAck(const QByteArray& data);

private:
	std::unique_ptr<QUdpSocket> _sock;
	QHostAddress				_sender;
	quint16						_senderPort;
};


using udp_server_ptr = std::unique_ptr<SimpleUdpServer>;
