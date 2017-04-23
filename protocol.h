#ifndef PROTOCOL_H
#define PROTOCOL_H

class IProtocol
{
public:
	virtual ~IProtocol() {}
	virtual int Open(const char *url) = 0;
	virtual int Close() = 0;
	virtual int Read(void *buffer, unsigned size) = 0;
};

#endif
