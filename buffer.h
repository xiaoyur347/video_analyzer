class BitBuffer
{
public:
	BitBuffer();
	void Reset(unsigned char *buffer, unsigned size);
	unsigned GetOneBit();
	unsigned GetBit(unsigned bits);
	unsigned GetByte(unsigned byte);
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
