#include "buffer.h"
#include <stdlib.h>
BitBuffer::BitBuffer()
	:mBuffer(NULL),
	 mBytes(0),
	 mBits(0),
	 mBitOffset(0),
	 mCurrentByte(0),
	 mCurrentBit(0)
{

}

void BitBuffer::Reset(unsigned char *buffer, unsigned size)
{
	mBuffer = buffer;
	mBytes = size;
	mBits = size * 8;
	mBitOffset = 0;
	mCurrentByte = 0;
	mCurrentBit = 0;
}

void BitBuffer::addBitOffset(unsigned bits)
{
	mBitOffset += bits;
	mCurrentByte = mBitOffset / 8;
	mCurrentBit = mBitOffset % 8;
}

unsigned BitBuffer::getBit(unsigned bits)
{
	unsigned ret = mBuffer[mCurrentByte];
	ret = (ret >> ((8 - bits) - mCurrentBit)) & ((1 << bits) - 1);
	return ret;
}

unsigned BitBuffer::GetOneBit()
{
	unsigned ret = getBit(1);
	addBitOffset(1);
	return ret;
}
unsigned BitBuffer::GetBit(unsigned bits)
{
	unsigned ret = 0;
	while (bits > 0)
	{
		unsigned copy_bits = 8 - mCurrentBit;
		if (bits < copy_bits)
		{
			copy_bits = bits;
		}

		ret = (ret << copy_bits) | getBit(copy_bits);
		addBitOffset(copy_bits);
		bits -= copy_bits;
	}
	return ret;
}
unsigned BitBuffer::GetByte(unsigned byte)
{
	return GetBit(byte * 8);
}

uint64_t BitBuffer::Get64BitBit(unsigned bits)
{
	uint64_t ret = 0;
	while (bits > 0)
	{
		unsigned copy_bits = 8 - mCurrentBit;
		if (bits < copy_bits)
		{
			copy_bits = bits;
		}

		ret = (ret << copy_bits) | getBit(copy_bits);
		addBitOffset(copy_bits);
		bits -= copy_bits;
	}
	return ret;
}

uint64_t BitBuffer::Get64BitByte(unsigned byte)
{
	uint64_t ret = 0;
	for (unsigned i = 0; i < byte; ++i)
	{
		ret = ret << 8 | GetBit(8);
	}
	return ret;
}

void BitBuffer::SkipBit(unsigned bits)
{
	if (bits != 0)
	{
		addBitOffset(bits);
	}
}

void BitBuffer::SkipByte(unsigned byte)
{
	SkipBit(8 * byte);
}

bool BitBuffer::IsEmpty() const
{
	return mBitOffset >= mBits;
}

unsigned BitBuffer::GetUEV()
{
	int leadingZeroBits = -1;
	unsigned b;
	for (b = 0; b == 0; leadingZeroBits++)
	{
		b = GetBit(1);
	}
	if (leadingZeroBits == 0)
	{
		return 0;
	}
	return (2 << (leadingZeroBits - 1)) - 1 + GetBit(leadingZeroBits);
}

int BitBuffer::GetSEV()
{
	unsigned k = GetUEV();
	if (k == 0)
	{
		return 0;
	}
	if (k % 2 == 0)
	{
		return -((k+1)/2);
	}
	return (k+1)/2;
}
