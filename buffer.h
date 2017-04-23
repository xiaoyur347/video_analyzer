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
	uint64_t Get64BitBit(unsigned bits);
	uint64_t Get64BitByte(unsigned byte);
	void SkipBit(unsigned bits);
	void SkipByte(unsigned byte);
	void SkipTrailing();
	void RewindBit(unsigned bits);
	bool IsEmpty() const;
	unsigned GetUEV();
	int GetSEV();
	void Dump();
private:
	void addBitOffset(unsigned bits);
	void minusBitOffset(unsigned bits);
	unsigned getBit(unsigned bits);
	unsigned char *mBuffer;
	unsigned mBytes;
	unsigned mBits;
	unsigned mBitOffset;

	unsigned mCurrentByte;
	unsigned mCurrentBit;
};

#endif
