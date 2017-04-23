#include "debug.h"
#include "file.h"
#include "flv.h"
#include <string.h>

void analyze_ts(IProtocol *protocol);

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		return 1;
	}
	const char *url = argv[1];
	IProtocol *protocol = NULL;
	if (strncmp(url, "/", 1) == 0 || strncmp(url, "~", 1) == 0
		|| strncmp(url, ".", 1) == 0)
	{
		protocol = new File();
	}

	if (protocol == NULL)
	{
		return 2;
	}
	if (protocol->Open(argv[1]) < 0)
	{
		LOG_ERROR("open fail");
		return 1;
	}
	if (strstr(url, ".ts") != NULL)
	{
		analyze_ts(protocol);
	}
	else if (strstr(url, ".flv") != NULL)
	{
		FlvDemuxer demuxer(protocol);
		demuxer.Run();
	}
	protocol->Close();
	delete protocol;
	return 0;
}
