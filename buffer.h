#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
class BitBuffer
{
public:
	BitBuffer();
	void Reset(unsigned char *buffer, unsigned size);
	unsigned GetOneBit();
	unsigned GetBit(unsigned bits);
	unsigned GetByte(unsigned byte);
	uint64_t Get64BitByte(unsigned byte);
private:
	void addBitOffset(unsigned bits);
	unsigned getBit(unsigned bits);
	unsigned char *mBuffer;
	unsigned mBytes;
	unsigned mBits;
	unsigned mBitOffset;

	unsigned mCurrentByte;
	unsigned mCurrentBit;
};

#endif
