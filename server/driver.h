#pragma once

#include <QObject>
#include <unordered_map>
#include "fileprocessor.h"
#include "server.h"



class ServerDriver : public QObject
{
	Q_OBJECT

public:
	ServerDriver(int port, QObject* parent = nullptr);

private slots:
	void onMessage(const QNetworkDatagram& package);

private:
	std::unordered_map<uint64_t, shared_file_processor> _processors;
	udp_server_ptr										_server;
};
