#include "ts.h"

#include "debug.h"

#include <string.h>
#include <unistd.h>

#define STREAM_TYPE_VIDEO_MPEG1     0x01
#define STREAM_TYPE_VIDEO_MPEG2     0x02
#define STREAM_TYPE_AUDIO_MPEG1     0x03
#define STREAM_TYPE_AUDIO_MPEG2     0x04
#define STREAM_TYPE_PRIVATE_SECTION 0x05
#define STREAM_TYPE_PRIVATE_DATA    0x06
#define STREAM_TYPE_AUDIO_AAC       0x0f
#define STREAM_TYPE_AUDIO_AAC_LATM  0x11
#define STREAM_TYPE_VIDEO_MPEG4     0x10
#define STREAM_TYPE_METADATA        0x15
#define STREAM_TYPE_VIDEO_H264      0x1b
#define STREAM_TYPE_VIDEO_HEVC      0x24
#define STREAM_TYPE_VIDEO_CAVS      0x42
#define STREAM_TYPE_VIDEO_VC1       0xea
#define STREAM_TYPE_VIDEO_DIRAC     0xd1

#define STREAM_TYPE_AUDIO_AC3       0x81
#define STREAM_TYPE_AUDIO_DTS       0x82
#define STREAM_TYPE_AUDIO_TRUEHD    0x83
#define STREAM_TYPE_AUDIO_EAC3      0x87

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

void TsFile::SDT::Analyze(BitBuffer &bits)
{
	table_id = bits.GetByte(1);
	section_syntax_indicator = bits.GetOneBit();
	reserved_1 = bits.GetOneBit();
	reserved_2 = bits.GetBit(2);
	section_length = bits.GetBit(12);

	transport_stream_id = bits.GetByte(2);

	reserved_3 = bits.GetBit(2);
	version_number = bits.GetBit(5);
	current_next_indicator = bits.GetOneBit();
	section_number = bits.GetByte(1);
	last_section_number = bits.GetByte(1);
	original_network_id = bits.GetByte(2);
	reserved_4 = bits.GetByte(1);
	for (int i = 0; i < section_length - 8 - 4;)
	{
		SDTService service;
		service.service_id = bits.GetByte(2);
		bits.GetBit(6);
		service.EIT_schedule_flag = bits.GetOneBit();
		service.EIT_present_following_flag = bits.GetOneBit();
		service.running_status = bits.GetBit(3);
		service.freed_CA_mode = bits.GetOneBit();
		service.descriptors_loop_length = bits.GetBit(12);
		LOG_WARN("service.descriptors_loop_length=%u", service.descriptors_loop_length);
#if 0
		for (unsigned j = 0; j < service.descriptors_loop_length; j++)
		{
			service.descriptor.push_back(bits.GetByte(1));
		}
#else
		if (service.descriptors_loop_length > 0)
		{
			service.descriptor_tag = bits.GetByte(1);
			service.descriptor_length = bits.GetByte(1);
			service.service_type = bits.GetByte(1);
			service.service_provider_name_length = bits.GetByte(1);
			for (unsigned j = 0; j < service.service_provider_name_length; j++)
			{
				service.service_provider_name.append(1, (char)bits.GetByte(1));
			}
			service.service_name_length = bits.GetByte(1);
			for (unsigned j = 0; j < service.service_name_length; j++)
			{
				service.service_name.append(1, (char)bits.GetByte(1));
			}
			LOG_INFO("tag=%x,length=%x,service_type=%u,provider=[%u]%s,name=[%u]%s",
				service.descriptor_tag,
				service.descriptor_length,
				service.service_type,
				service.service_provider_name_length,
				service.service_provider_name.c_str(),
				service.service_name_length,
				service.service_name.c_str());
		}
#endif
		i += 5 + service.descriptors_loop_length;
	}

	CRC_32 = bits.GetByte(4);
	LOG_WARN("SDT CRC %x", CRC_32);
	//Dump();
}

void TsFile::SDT::Dump()
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

void TsFile::PAT::Analyze(BitBuffer &bits)
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
	//section_length = sizeof(transport_stream_id->last_section_number+PATProgram vector+CRC32)
	//so, section_length = 5 + PATProgram vector + 4
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
			PATProgram program;
			program.program_map_PID = pid;
			program.program_number = program_num;
			LOG_WARN("pmt pid=%u,program_number=%u",
				program.program_map_PID,
				program.program_number);
			vecProgram.push_back(program);
		}
	}

	CRC_32 = bits.GetByte(4);
	LOG_WARN("PAT CRC %x", CRC_32);
	//Dump();
}

void TsFile::PAT::Dump()
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

const char *TsFile::PMT::GetTypeName(unsigned type) const
{
	const char *name = "";
	switch (type)
	{
		case STREAM_TYPE_VIDEO_MPEG1:
			name = "[video]mpeg1";
			break;
		case STREAM_TYPE_VIDEO_MPEG2:
			name = "[video]mpeg2";
			break;
		case STREAM_TYPE_AUDIO_MPEG1:
			name = "[audio]mp1";
			break;
		case STREAM_TYPE_AUDIO_MPEG2:
			name = "[audio]mp2";
			break;
		case STREAM_TYPE_PRIVATE_SECTION:
			name = "private section";
			break;
		case STREAM_TYPE_PRIVATE_DATA:
			name = "private data";
			break;
		case STREAM_TYPE_AUDIO_AAC:
			name = "[audio]aac";
			break;
		case STREAM_TYPE_AUDIO_AAC_LATM:
			name = "[audio]aac latm";
			break;
		case STREAM_TYPE_VIDEO_MPEG4:
			name = "[video]mpeg4";
			break;
		case STREAM_TYPE_METADATA:
			name = "metadata";
			break;
		case STREAM_TYPE_VIDEO_H264:
			name = "[video]h264";
			break;
		case STREAM_TYPE_VIDEO_HEVC:
			name = "[video]h265";
			break;
		case STREAM_TYPE_VIDEO_CAVS:
			name = "[video]cavs";
			break;
		case STREAM_TYPE_VIDEO_VC1:
			name = "[video]vc1";
			break;
		case STREAM_TYPE_VIDEO_DIRAC:
			name = "[video]dirac";
			break;
		case STREAM_TYPE_AUDIO_AC3:
			name = "[audio]ac3";
			break;
		case STREAM_TYPE_AUDIO_DTS:
			name = "[audio]dts";
			break;
		case STREAM_TYPE_AUDIO_TRUEHD:
			name = "[audio]truehd";
			break;
		case STREAM_TYPE_AUDIO_EAC3:
			name = "[audio]eac3";
			break;
	}
	return name;
}

void TsFile::PMT::Analyze(BitBuffer &bits)
{
	table_id = bits.GetByte(1);
	section_syntax_indicator = bits.GetOneBit();
	zero = bits.GetOneBit();
	reserved_1 = bits.GetBit(2);
	section_length = bits.GetBit(12);
	program_number = bits.GetByte(2);
	reserved_2 = bits.GetBit(2);
	version_number = bits.GetBit(5);
	current_next_indicator = bits.GetOneBit();
	section_number = bits.GetByte(1);
	last_section_number = bits.GetByte(1);
	reserved_3 = bits.GetBit(3);
	PCR_PID = bits.GetBit(13);
	reserved_4 = bits.GetBit(4);
	program_info_length = bits.GetBit(12);

	if (program_info_length > 0)
	{
		for (unsigned i = 0; i < program_info_length; i++)
		{
			bits.GetByte(1);
		}
	}

	LOG_INFO("PCR_PID=%u,section_length=%u,program_info_length=%u",
		PCR_PID, section_length, program_info_length);

	//9 is program_number->program_info_length, 4 is CRC32
	for (unsigned i = 0; i < section_length - 9 - program_info_length - 4;)
	{
		PMTStream stream;
		stream.stream_type = bits.GetByte(1);
		bits.GetBit(3);
		stream.elementary_PID = bits.GetBit(13);
		bits.GetBit(4);
		stream.ES_info_length = bits.GetBit(12);
		stream.descriptor = 0;
		for (unsigned j = 0; j < stream.ES_info_length; j++)
		{
			stream.descriptor |= bits.GetByte(1);
		}
		LOG_WARN("PMT %u,type=%u,%s", stream.elementary_PID, stream.stream_type,
			GetTypeName(stream.stream_type));
		vecStream.push_back(stream);

		i += 5 + stream.ES_info_length;
	}

	CRC_32 = bits.GetByte(4);
	LOG_WARN("PMT CRC %x", CRC_32);
	Dump();
}

void TsFile::PMT::Dump()
{
	LOG_DEBUG("program_number=%u,"
		"version_number=%u,"
		"current_next_indicator=%u,"
		"section_number=%u,"
		"last_section_number=%u,",
		program_number,
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

bool TsFile::IsPMT(unsigned pid) const
{
	for (unsigned i = 0; i < mPAT.vecProgram.size(); i++)
	{
		if (mPAT.vecProgram[i].program_map_PID == pid)
		{
			return true;
		}
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
	else if (header.pid == 0x11)
	{
		//SDT
		mSDT.Analyze(mBits);
	}
	else
	{
		if (IsPMT(header.pid))
		{
			mPMT.Analyze(mBits);
		}
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
