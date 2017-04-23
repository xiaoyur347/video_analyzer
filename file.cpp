#include "file.h"
#include <fcntl.h> //for open
#include <unistd.h> //for close

File::File()
	:mFd(-1)
{

}

File::~File()
{
	File::Close();
}

int File::Open(const char *url)
{
	mFd = open(url, 0644);
	if (mFd >= 0)
	{
		return 0;
	}
	return -1;
}

int File::Close()
{
	if (mFd >= 0)
	{
		close(mFd);
	}
	return 0;
}

int File::Read(void *buffer, unsigned size)
{
	return read(mFd, buffer, size);
}

int File::Write(const void *buffer, unsigned size)
{
	return write(mFd, buffer, size); //0644 will avoid write
}
