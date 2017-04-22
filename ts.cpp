#include "ts.h"

#include "debug.h"

#include <string.h>
#include <unistd.h>

const char *TsFile::PacketHeader::GetPidName() const
{
	switch (pid)
	{
		case 0:
			return "PAT";
		case 1:
			return "CAT";
		case 2:
			return "TSDT";
		case 0x12:
			return "EIT,ST";
		case 0x13:
			return "RST,ST";
		case 0x14:
			return "TDT,TOT,ST";
		default:
			return "";
	}
}

void TsFile::PacketHeader::Dump()
{
	LOG_DEBUG("%sstart_indicator=%d,"
		"%spid=(%s)%d,"
		"%sadaptation_field_control=%d,"
		"continuity_counter=%d",
		transport_error_indicator != 0 ? "error_indicator=1," : "",
		payload_unit_start_indicator,
		transport_priority != 0 ? "priority=1," : "",
		GetPidName(), pid,
		transport_scrambling_control != 0 ? "scrambling_control=1," : "",
		adaptation_field_control,
		continuity_counter);
}

TsFile::TsFile(int fd)
	:mFd(fd),
	 mPacket(0)
{
	memset(mBuffer, 0, sizeof(mBuffer));
}

TsFile::~TsFile()
{

}

bool TsFile::ReadPacket()
{
	while (true)
	{
		int ret = read(mFd, mBuffer, 1);
		if (ret != 1)
		{
			return false;
		}
		//0x47 is the sync byte
		if (mBuffer[0] != 0x47)
		{
			continue;
		}
		ret = read(mFd, mBuffer + 1, TS_PACKET_SIZE - 1);
		if (ret != TS_PACKET_SIZE - 1)
		{
			return false;
		}
		mBits.Reset(mBuffer, TS_PACKET_SIZE);
		LOG_DEBUG("get packet %d", mPacket);
		++mPacket;
		return true;
	}
	return false;
}

bool TsFile::AnalyzePacket()
{
	PacketHeader header;

	header.sync_byte = mBits.GetByte(1);

	header.transport_error_indicator = mBits.GetOneBit();
	header.payload_unit_start_indicator = mBits.GetOneBit();
	header.transport_priority = mBits.GetOneBit();
	header.pid = mBits.GetBit(13);

	header.transport_scrambling_control = mBits.GetBit(2);
	header.adaptation_field_control = mBits.GetBit(2);
	header.continuity_counter = mBits.GetBit(4);
	header.Dump();

	if (header.pid == 0)
	{
		//PAT
		AnalyzePAT();
	}
	return true;
}

bool TsFile::AnalyzePAT()
{
	TS_PAT packet;
	packet.table_id = mBits.GetByte(1);
	packet.section_syntax_indicator = mBits.GetOneBit();
	packet.zero = mBits.GetOneBit();
	packet.reserved_1 = mBits.GetBit(2);
	packet.section_length = mBits.GetBit(12);

	packet.transport_stream_id = mBits.GetByte(2);

	packet.reserved_2 = mBits.GetBit(2);
	packet.version_number = mBits.GetBit(5);
	packet.current_next_indicator = mBits.GetOneBit();
	packet.section_number = mBits.GetByte(1);
	packet.last_section_number = mBits.GetByte(1);

	for (int i = 0; i < packet.section_length; i++)
	{
		unsigned program_num = mBits.GetByte(2);
		packet.reserved_3 = mBits.GetBit(3);

		packet.network_PID = 0;
		unsigned pid = mBits.GetBit(13);
		if (program_num == 0)
		{
			packet.network_PID = pid;
			LOG_DEBUG("network_PID=%u", packet.network_PID);
		}
		else
		{
			TS_PAT_Program PAT_program;
			PAT_program.program_map_PID = pid;
			PAT_program.program_number = program_num;
			packet.program.push_back(PAT_program);
		}
	}

	packet.CRC_32 = mBits.GetByte(4);

	return true;
}

void analyze_ts(int fd)
{
	TsFile file(fd);
	while (file.ReadPacket())
	{
		file.AnalyzePacket();
		if (file.GetPacketNum() > 100)
		{
			break;
		}
	}
}
