#ifndef FILE_H
#define FILE_H

#include "protocol.h"

class File : public IProtocol
{
public:
	File();
	~File();
	int Open(const char *url);
	int Close();
	int Read(void *buffer, unsigned size);
	int Write(const void *buffer, unsigned size);
private:
	int mFd;
};

#endif
