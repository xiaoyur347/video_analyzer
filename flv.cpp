#include "flv.h"

#include "debug.h"

bool FlvDemuxer::FlvHeader::Analyze(BitBuffer &bits)
{
	signature = bits.GetByte(3);
	if (signature != 0x464c56)
	{
		LOG_ERROR("Error signature %u", signature);
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
			return "metadata";
		default:
			return "unknown";
	}
}

const char *FlvDemuxer::GetFrameType(unsigned frame_type) const
{
	switch (frame_type)
	{
		case 1:
			return "key frame";
		case 2:
			return "non-key frame";
		case 3:
			return "H.263 disposable frame";
		case 4:
			return "generated key frame";
		case 5:
			return "one byte frame seeking instruction";
		default:
			return "unknown";
	}
}

const char *FlvDemuxer::GetVideoCodecName(unsigned codec_id) const
{
	switch (codec_id)
	{
		case 0:
			return "RGB";
		case 1:
			return "run-length";
		case 2:
			return "Sorenson's H.263";
		case 3:
			return "Screen 1";
		case 4:
			return "On2 TrueMotion VP6";
		case 5:
			return "VP6 with alpha";
		case 6:
			return "Screen 2";
		case 7:
			return "MP4 H.264";
		case 8:
			return "ITU H.263";
		case 9:
			return "MPEG-4 ASP";
		default:
			return "unknown";
	}
}

const char *FlvDemuxer::GetAudioCodecName(unsigned codec_id) const
{
	//https://wuyuans.com/2012/08/flv-format/
	switch (codec_id)
	{
		case 0:
			return "native PCM";
		case 1:
			return "ADPCM";
		case 2:
			return "MP3";
		case 3:
			return "PCMLE";
		case 4:
			return "Nellymoser 16-kHz mono";
		case 5:
			return "Nellymoser 8-kHz mono";
		case 6:
			return "Nellymoser";
		case 7:
			return "ALAW";
		case 8:
			return "ULAW";
		case 10:
			return "AAC";
		case 11:
			return "SPEEX";
		case 14:
			return "MP3 8KHz";
		case 15:
			return "Device-specific sound";
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
			codec_id, GetVideoCodecName(codec_id));
		if (codec_id == 7)
		{
			unsigned avc_packet_type = mBits.GetByte(1);
			unsigned composition_time = mBits.GetByte(3);
			LOG_WARN("avc_packet_type=%u,composition_time=%u",
				avc_packet_type,
				composition_time);
		}
	}
	else if (tag_type == 8)
	{
		//audio one byte head
		unsigned codec_id = mBits.GetBit(4);
		unsigned sampling_frequency_index = mBits.GetBit(2);
		unsigned sampling_size = mBits.GetOneBit();
		unsigned stereo = mBits.GetOneBit();
		LOG_WARN("codec_id=%u(%s),sampling_frequency_index=%u,"
			"sampling_size=%u,stereo=%u",
			codec_id, GetAudioCodecName(codec_id),
			sampling_frequency_index, sampling_size, stereo);
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
