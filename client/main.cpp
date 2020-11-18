#include "driver.h"

boost::asio::io_service service;


#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

void start_service()
{
	boost::shared_ptr<client_driver> driver{ new client_driver{service, "127.0.0.1", 7755} };
	driver->initialaze(boost::filesystem::current_path() / "testdata", MAX_PACKAGE_SIZE, REPEAT_PACK_VALUE);
#ifndef MULTITHREAD
	driver->run(client_driver::mode::singlethread);	
#else 
	driver->run(client_driver::mode::multithread);
#endif
	service.stop();
}


int main(int argc, char** argv)
{
	std::shared_ptr<boost::asio::io_service::work> work(new boost::asio::io_service::work(service));
	service.post(boost::bind(start_service));
	
	const int THREADS_CNT = 10;
	boost::thread_group	tg;
	for (int i = 0; i < THREADS_CNT; ++i)
		tg.create_thread(boost::bind(&boost::asio::io_service::run, &service));			
	tg.join_all();

	return 0;
}
