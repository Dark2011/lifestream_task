
#include "driver.h"



boost::asio::io_service service;


//message_hdr generate_ack(const std::vector<uint8_t>& request)
//{
//	std::lock_guard<std::mutex> lg{ gl_lock };
//	
//	message_hdr* reqHdr = ((message_hdr*)&request[0]);
//	message_hdr resp;
//
//
//	uint64_t* id = (uint64_t*)reqHdr->_id;
//	auto res = receivedPacks[*id].insert(reqHdr->_seqNumber);
//	if (res.second)
//		++counters[*id];
//	else
//	{
//		std::cout << "Thread id: " << std::this_thread::get_id() << std::endl;
//		std::cout << "Package with sequence number: " << reqHdr->_seqNumber << " has already been received" << std::endl;
//	}
//	
//
//	memcpy(resp._id, reqHdr->_id, sizeof(uint8_t) * 8);
//	resp._seqNumber = reqHdr->_seqNumber;
//	resp._type = 0;
//	resp._seqTotal = counters[*id];
//	
//	return resp;
//
//


void start_service()
{
	std::shared_ptr<client_driver> driver{ new client_driver{service, "127.0.0.1", 7755} };
	driver->initialaze(std::filesystem::current_path() / "testdata", MAX_PACKAGE_SIZE, REPEAT_PACK_VALUE);
	driver->run(client_driver::mode::siglethread);
	service.stop();
}


int main(int argc, char** argv)
{
	//std::unique_ptr<boost::asio::io_service::work> work{ new boost::asio::io_service::work(service) };
	service.post(boost::bind(start_service));
	
	const int THREADS_CNT = 5;
	boost::thread_group	tg;
	for (int i = 0; i < THREADS_CNT; ++i)
		tg.create_thread(boost::bind(&boost::asio::io_service::run, &service));			
	tg.join_all();

	return 0;
}
