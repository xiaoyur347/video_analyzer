#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "ts.h"
#include "pes.h"
#include "type.h"

#include "debug.h"

#include <string.h>
#include <unistd.h>

void TsFile::AdaptionField::Analyze(BitBuffer &bits)
{
	adaption_field_length = bits.GetByte(1);
	discontinuity_indicator = bits.GetOneBit();
	random_access_indicator = bits.GetOneBit();
	elementary_stream_priority_indicator = bits.GetOneBit();
	PCR_flag = bits.GetOneBit();
	OPCR_flag = bits.GetOneBit();
	splicing_point_flag = bits.GetOneBit();
	transport_private_data_flag = bits.GetOneBit();
	adaption_field_extension_flag = bits.GetOneBit();

	//optional
	if (PCR_flag == 1)
	{
		PCR = bits.Get64BitBit(33) * 300; //33bits
		bits.GetBit(6);
		PCR += bits.GetBit(9); //extension
	}
	if (OPCR_flag == 1)
	{
		OPCR = bits.Get64BitByte(6) * 300; //33bits
		bits.GetBit(6);
		OPCR += bits.GetBit(9); //extension
	}
	LOG_DEBUG("PCR[%u]=%" PRIu64, PCR_flag, PCR);
	bits.SkipByte(adaption_field_length - 1 - PCR_flag * 6 + OPCR_flag * 6);
}

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

	if (adaptation_field_control != 1)
	{
		adaption = new AdaptionField();
		adaption->Analyze(bits);
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

void TsFile::SDT::Analyze(BitBuffer &bits, unsigned payload)
{
	if (payload)
	{
		unsigned pointer_field = bits.GetByte(1); //skip 00
		bits.SkipByte(pointer_field);
	}

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
		//LOG_WARN("service.descriptors_loop_length=%u", service.descriptors_loop_length);
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
	//LOG_WARN("SDT CRC %x", CRC_32);
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

void TsFile::PAT::Analyze(BitBuffer &bits, unsigned payload)
{
	if (payload)
	{
		unsigned pointer_field = bits.GetByte(1); //skip 00
		bits.SkipByte(pointer_field);
	}

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
	//LOG_WARN("PAT CRC %x", CRC_32);
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

bool TsFile::PMT::IsVideoStreamType(unsigned stream_type) const
{
	switch (stream_type) {
		case STREAM_TYPE_VIDEO_MPEG1:
		case STREAM_TYPE_VIDEO_MPEG2:
		case STREAM_TYPE_VIDEO_MPEG4:
		case STREAM_TYPE_VIDEO_H264:
		case STREAM_TYPE_VIDEO_HEVC:
		case STREAM_TYPE_VIDEO_CAVS:
		case STREAM_TYPE_VIDEO_VC1:
		case STREAM_TYPE_VIDEO_DIRAC:
			return true;
	}
	return false;
}
bool TsFile::PMT::IsAudioStreamType(unsigned stream_type) const
{
	switch (stream_type) {
		case STREAM_TYPE_AUDIO_MPEG1:
		case STREAM_TYPE_AUDIO_MPEG2:
		case STREAM_TYPE_AUDIO_AAC:
		case STREAM_TYPE_AUDIO_AAC_LATM:
		case STREAM_TYPE_AUDIO_AC3:
		case STREAM_TYPE_AUDIO_DTS:
		case STREAM_TYPE_AUDIO_TRUEHD:
		case STREAM_TYPE_AUDIO_EAC3:
			return true;
	}
	return false;
}

unsigned TsFile::PMT::GetVideoPid() const
{
	if (vecStream.empty())
	{
		return 0;
	}
	for (unsigned i = 0; i < vecStream.size(); i++)
	{
		if (IsVideoStreamType(vecStream[i].stream_type))
		{
			return vecStream[i].elementary_PID;
		}
	}
	return 0;
}
unsigned TsFile::PMT::GetAudioPid() const
{
	if (vecStream.empty())
	{
		return 0;
	}
	for (unsigned i = 0; i < vecStream.size(); i++)
	{
		if (IsAudioStreamType(vecStream[i].stream_type))
		{
			return vecStream[i].elementary_PID;
		}
	}
	return 0;
}
unsigned TsFile::PMT::GetVideoStreamType() const
{
	if (vecStream.empty())
	{
		return 0;
	}
	for (unsigned i = 0; i < vecStream.size(); i++)
	{
		if (IsVideoStreamType(vecStream[i].stream_type))
		{
			return vecStream[i].stream_type;
		}
	}
	return 0;
}
unsigned TsFile::PMT::GetAudioStreamType() const
{
	if (vecStream.empty())
	{
		return 0;
	}
	for (unsigned i = 0; i < vecStream.size(); i++)
	{
		if (IsAudioStreamType(vecStream[i].stream_type))
		{
			return vecStream[i].stream_type;
		}
	}
	return 0;
}

void TsFile::PMT::Analyze(BitBuffer &bits, unsigned payload)
{
	if (payload)
	{
		unsigned pointer_field = bits.GetByte(1); //skip 00
		bits.SkipByte(pointer_field);
	}

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
		bits.SkipByte(program_info_length);
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
	//LOG_WARN("PMT CRC %x", CRC_32);
	//Dump();
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

TsFile::TsFile(IProtocol *protocol)
	:mProtocol(protocol),
	 mPacket(0),
	 mVideoPid(0),
	 mAudioPid(0),
	 mVideoStreamType(0),
	 mAudioStreamType(0)
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
		int ret = mProtocol->Read(mBuffer, 1);
		if (ret != 1)
		{
			return false;
		}
		//0x47 is the sync byte
		if (mBuffer[0] != 0x47)
		{
			continue;
		}
		ret = mProtocol->Read(mBuffer + 1, TS_PACKET_SIZE - 1);
		if (ret != TS_PACKET_SIZE - 1)
		{
			return false;
		}
		mBits.Reset(mBuffer, TS_PACKET_SIZE);
		printf("\n");
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
		mPAT.Analyze(mBits, header.payload_unit_start_indicator);
	}
	else if (header.pid == 0x11)
	{
		//SDT
		mSDT.Analyze(mBits, header.payload_unit_start_indicator);
	}
	else if (mVideoPid != 0 && header.pid == mVideoPid)
	{
		LOG_WARN("Video frame");
		if (header.payload_unit_start_indicator == 1)
		{
			if (!mVideoBuffer.empty())
			{
				//delay the analyze
				PES pes;
				BitBuffer bits;
				LOG_WARN("last video pes %u", (unsigned)mVideoBuffer.size());
				bits.Reset(&mVideoBuffer[0], mVideoBuffer.size());
				pes.Analyze(bits, mVideoStreamType);
			}
			mVideoBuffer.clear();
			while (!mBits.IsEmpty())
			{
				mVideoBuffer.push_back(mBits.GetByte(1));
			}
		}
		else
		{
			while (!mBits.IsEmpty())
			{
				mVideoBuffer.push_back(mBits.GetByte(1));
			}
		}
	}
	else if (mAudioPid != 0 && header.pid == mAudioPid)
	{
		LOG_WARN("Audio frame");
		if (header.payload_unit_start_indicator == 1)
		{
			if (!mAudioBuffer.empty())
			{
				//delay the analyze
				PES pes;
				BitBuffer bits;
				LOG_WARN("last audio pes %u", (unsigned)mAudioBuffer.size());
				bits.Reset(&mAudioBuffer[0], mAudioBuffer.size());
				pes.Analyze(bits, mAudioStreamType);
			}
			mAudioBuffer.clear();
			while (!mBits.IsEmpty())
			{
				mAudioBuffer.push_back(mBits.GetByte(1));
			}
		}
		else
		{
			while (!mBits.IsEmpty())
			{
				mAudioBuffer.push_back(mBits.GetByte(1));
			}
		}
	}
	else
	{
		if (IsPMT(header.pid))
		{
			mPMT.Analyze(mBits, header.payload_unit_start_indicator);
			mVideoPid = mPMT.GetVideoPid();
			mAudioPid = mPMT.GetAudioPid();
			mVideoStreamType = mPMT.GetVideoStreamType();
			mAudioStreamType = mPMT.GetAudioStreamType();
		}
	}
	return true;
}

void analyze_ts(IProtocol* protocol)
{
	TsFile file(protocol);
	while (file.ReadPacket())
	{
		file.AnalyzePacket();
		if (file.GetPacketNum() > 1000)
		{
			break;
		}
	}
}
