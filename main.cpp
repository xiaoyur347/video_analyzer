#include <fcntl.h> //for open
#include <unistd.h> //for close
#include "debug.h"

void analyze_ts(int fd);

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		return 1;
	}
	int fd = open(argv[1], 0644);
	if (fd < 0)
	{
		LOG_ERROR("open fail");
		return 1;
	}
	analyze_ts(fd);
	close(fd);
	return 0;
}
