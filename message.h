#pragma once


#include <stdint.h>


#pragma pack (push, 1)
struct message_hdr
{
	uint32_t _seqNumber;
	uint32_t _seqTotal;
	uint8_t  _type;
	uint8_t	 _id[8];
};
#pragma pack (pop)


const int ACK = 0;
const int PUT = 1;


const int MAX_PACKAGE_SIZE = 1472;
const int REPEAT_PACK_VALUE = 3;
const int ID_LENGTH = 8;