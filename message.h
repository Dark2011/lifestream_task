#pragma once


#include <stdint.h>

const int ID_LENGTH = 8;
const int ACK = 0;
const int PUT = 1;

const int MAX_PACKAGE_SIZE = 1472;
const int REPEAT_PACK_VALUE = 3;


#pragma pack (push, 1)
struct message_hdr
{
	uint32_t _seqNumber;
	uint32_t _seqTotal;
	uint8_t  _type;
	uint8_t	 _id[ID_LENGTH];
};
#pragma pack (pop)


const int HEADER_SIZE = sizeof(message_hdr);


inline uint32_t crc32c(uint32_t crc, const unsigned char* buf, size_t len)
{	
	crc = ~crc;
	while (len--)
	{
		crc ^= *buf++;
		for (int k = 0; k < 8; ++k)
			crc = crc & 1 ? (crc >> 1) ^ 0x82f63b78 : crc >> 1;
	}
	return ~crc;
}
