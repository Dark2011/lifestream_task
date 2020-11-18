#include "driver.h"

boost::asio::io_service service;


#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

void start_service(const std::string& addr, uint16_t port)
{
	boost::shared_ptr<client_driver> driver{ new client_driver{service, addr, port} };
	driver->initialaze(boost::filesystem::current_path() / "upload", MAX_PACKAGE_SIZE, REPEAT_PACK_VALUE);
#ifndef MULTITHREAD
	driver->run(client_driver::mode::singlethread);	
#else 
	driver->run(client_driver::mode::multithread);
#endif
	service.stop();
}


int main(int argc, char** argv)
{
	std::string server_addr = "127.0.0.1";
	uint16_t server_port = 7755;

	if (argc == 3)
	{
		server_addr = std::string(argv[1]);
		server_port = atoi(argv[2]);
	}

	std::shared_ptr<boost::asio::io_service::work> work(new boost::asio::io_service::work(service));
	service.post(boost::bind(start_service, server_addr, server_port));
	

	const int THREADS_CNT = 3;
	boost::thread_group	tg;
	for (int i = 0; i < THREADS_CNT; ++i)
		tg.create_thread(boost::bind(&boost::asio::io_service::run, &service));			
	tg.join_all();

	return 0;
}
