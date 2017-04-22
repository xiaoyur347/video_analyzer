#include "ts.h"

#include "debug.h"

#include <string.h>
#include <unistd.h>

const char *TsFile::PacketHeader::GetPidName() const
{
	switch (pid)
	{
		case 0:
			return "PAT"; //important
		case 1:
			return "CAT";
		case 2:
			return "TSDT";
		case 0x11:
			return "SDT"; //important
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

void TsFile::PacketHeader::Analyze(BitBuffer &bits)
{
	sync_byte = bits.GetByte(1);

	transport_error_indicator = bits.GetOneBit();
	payload_unit_start_indicator = bits.GetOneBit();
	transport_priority = bits.GetOneBit();
	pid = bits.GetBit(13);

	transport_scrambling_control = bits.GetBit(2);
	adaptation_field_control = bits.GetBit(2);
	continuity_counter = bits.GetBit(4);

	if (payload_unit_start_indicator == 1)
	{
		bits.GetByte(1);
	}
}

void TsFile::PacketHeader::Dump()
{
	LOG_DEBUG("%sstart_indicator=%u,"
		"%spid=(%s)%u,"
		"%sadaptation_field_control=%u,"
		"continuity_counter=%u",
		transport_error_indicator != 0 ? "error_indicator=1," : "",
		payload_unit_start_indicator,
		transport_priority != 0 ? "priority=1," : "",
		GetPidName(), pid,
		transport_scrambling_control != 0 ? "scrambling_control=1," : "",
		adaptation_field_control,
		continuity_counter);
}

void TsFile::TS_PAT::Analyze(BitBuffer &bits)
{
	table_id = bits.GetByte(1);
	section_syntax_indicator = bits.GetOneBit();
	zero = bits.GetOneBit();
	reserved_1 = bits.GetBit(2);
	section_length = bits.GetBit(12);

	transport_stream_id = bits.GetByte(2);

	reserved_2 = bits.GetBit(2);
	version_number = bits.GetBit(5);
	current_next_indicator = bits.GetOneBit();
	section_number = bits.GetByte(1);
	last_section_number = bits.GetByte(1);
	//section_length = sizeof(transport_stream_id->last_section_number+TS_PAT_Program vector+CRC32)
	//so, section_length = 5 + TS_PAT_Program vector + 4
	for (int i = 0; i < section_length - 5 - 4; i+=4)
	{
		unsigned program_num = bits.GetByte(2);
		reserved_3 = bits.GetBit(3);

		network_PID = 0;
		unsigned pid = bits.GetBit(13);
		if (program_num == 0)
		{
			network_PID = pid;
			LOG_DEBUG("network_PID=%u", network_PID);
		}
		else
		{
			TS_PAT_Program PAT_program;
			PAT_program.program_map_PID = pid;
			LOG_DEBUG("pmt pid=%u", PAT_program.program_map_PID);
			PAT_program.program_number = program_num;
			program.push_back(PAT_program);
		}
	}

	CRC_32 = bits.GetByte(4);
	Dump();
}

void TsFile::TS_PAT::Dump()
{
	LOG_DEBUG("section_syntax_indicator=%u,"
		"transport_stream_id=%u,"
		"version_number=%u,"
		"current_next_indicator=%u,"
		"section_number=%u,"
		"last_section_number=%u,",
		section_syntax_indicator, transport_stream_id,
		version_number,
		current_next_indicator,
		section_number,
		last_section_number);
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
		LOG_DEBUG("get packet %u", mPacket);
		++mPacket;
		return true;
	}
	return false;
}

bool TsFile::AnalyzePacket()
{
	PacketHeader header;
	header.Analyze(mBits);
	header.Dump();

	if (header.pid == 0)
	{
		//PAT
		mPAT.Analyze(mBits);
	}
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
