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

const char *FlvDemuxer::GetTagName(unsigned tag_type) const
{
	switch (tag_type)
	{
		case 8:
			return "audio";
		case 9:
			return "video";
		case 12:
			return "data";
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

	SkipByte(data_size);
	LOG_INFO("tag_type=%u(%s),data_size=%u,timestamp=%u,"
		"timestamp_extended=%u,stream_id=%u",
		tag_type, GetTagName(tag_type), data_size, timestamp,
		timestamp_extended, stream_id);

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
