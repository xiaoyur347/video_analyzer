#include "flv.h"

#include "debug.h"

bool FlvDemuxer::FlvHeader::Analyze(BitBuffer &bits)
{
	sync_byte = bits.GetByte(3);
	if (sync_byte != 0x464c56)
	{
		LOG_ERROR("Error sync_byte %u", sync_byte);
		return false;
	}

	version = bits.GetByte(1);
	if (version != 1)
	{
		LOG_ERROR("Error version %u", version);
		return false;
	}

	type_flags_reversed1 = bits.GetBit(5);
	type_flags_audio = bits.GetOneBit();
	type_flags_reversed2 = bits.GetOneBit();
	type_flags_video = bits.GetOneBit();
	data_offset = bits.GetByte(4);

	if (data_offset != 9)
	{
		LOG_ERROR("Error data_offset %u", data_offset);
		return false;
	}

	LOG_INFO("video %u, audio %u", type_flags_video, type_flags_audio);

	return true;
}

FlvDemuxer::FlvDemuxer(IProtocol *protocol)
	:mProtocol(protocol),
	 mPacket(0)
{

}

FlvDemuxer::~FlvDemuxer()
{

}

bool FlvDemuxer::ReadHeader()
{
	unsigned char head[9];
	mProtocol->Read(head, sizeof(head));
	mBits.Reset(head, sizeof(head));

	mHeader.Analyze(mBits);
	return true;
}

unsigned FlvDemuxer::ReadByte(unsigned size)
{
	unsigned ret = 0;
	for (;size > 0; --size)
	{
		unsigned char byte;
		mProtocol->Read(&byte, 1);
		ret = (ret << 8) | byte;
	}
	return ret;
}

void FlvDemuxer::SkipByte(unsigned size)
{
	for (;size > 0; --size)
	{
		unsigned char byte;
		mProtocol->Read(&byte, 1);
	}
}

void FlvDemuxer::ReadBuffer(unsigned size)
{
	mBuffer.clear();
	for (;size > 0; --size)
	{
		unsigned char byte;
		mProtocol->Read(&byte, 1);
		mBuffer.push_back(byte);
	}
	mBits.Reset(&mBuffer[0], mBuffer.size());
}

const char *FlvDemuxer::GetTagName(unsigned tag_type) const
{
	switch (tag_type)
	{
		case 8:
			return "audio";
		case 9:
			return "video";
		case 0x12:
			return "data";
		default:
			return "unknown";
	}
}

const char *FlvDemuxer::GetFrameType(unsigned frame_type) const
{
	switch (frame_type)
	{
		case 1:
			return "key";
		case 2:
			return "not_key";
		case 3:
			return "h263_not_key";
		case 4:
			return "server_key";
		case 5:
			return "info/command";
		default:
			return "unknown";
	}
}

const char *FlvDemuxer::GetCodecName(unsigned codec_id) const
{
	switch (codec_id)
	{
		case 1:
			return "JPEG";
		case 2:
			return "H263";
		case 3:
			return "ScreenVideo";
		case 4:
			return "VP6FLVVideo";
		case 5:
			return "VP6FLVAlphaVideo";
		case 6:
			return "ScreenV2Video";
		case 7:
			return "AVC";
		default:
			return "unknown";
	}
}

bool FlvDemuxer::ReadPacket()
{
	unsigned previous_tag_size[4];
	if (mProtocol->Read(previous_tag_size, 4) < 0)
	{
		return false;
	}

	unsigned char tag_type;
	if (mProtocol->Read(&tag_type, 1) < 0)
	{
		return false;
	}

	unsigned int data_size = ReadByte(3);
	unsigned int timestamp = ReadByte(3);
	unsigned int timestamp_extended = ReadByte(1);
	unsigned int stream_id = ReadByte(3);

	if (stream_id != 0)
	{
		LOG_ERROR("stream_id=%u", stream_id);
	}

	ReadBuffer(data_size);
	LOG_INFO("tag_type=%s,data_size=%u,timestamp=%u,"
		"timestamp_extended=%u",
		GetTagName(tag_type), data_size, timestamp,
		timestamp_extended);

	if (tag_type == 9)
	{
		//video
		unsigned frame_type = mBits.GetBit(4);
		unsigned codec_id = mBits.GetBit(4);
		LOG_WARN("frame_type=%u(%s),codec_id=%u(%s)",
			frame_type, GetFrameType(frame_type),
			codec_id, GetCodecName(codec_id));
	}
	else if (tag_type == 8)
	{
		//audio
	}

	++mPacket;

	return true;
}

void FlvDemuxer::Run()
{
	ReadHeader();
	while (ReadPacket())
	{
		if (GetPacketNum() > 100)
		{
			break;
		}
	}
}
