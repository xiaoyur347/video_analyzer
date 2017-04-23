#ifndef FLV_H
#define FLV_H

#include "protocol.h"
#include "buffer.h"
#include <vector>

class FlvDemuxer
{
public:
	struct FlvHeader
	{
		unsigned signature : 24;
		unsigned version : 8;
		unsigned type_flags_reversed1 : 5;
		unsigned type_flags_audio : 1;
		unsigned type_flags_reversed2: 1;
		unsigned type_flags_video : 1;
		unsigned data_offset;

		FlvHeader()
			:signature(0),
			version(0),
			type_flags_reversed1(0),
			type_flags_audio(0),
			type_flags_reversed2(0),
			type_flags_video(0),
			data_offset(0)
		{
		}

		bool Analyze(BitBuffer &bits);
	};
	explicit FlvDemuxer(IProtocol *protocol);
	~FlvDemuxer();
	bool ReadHeader();
	bool ReadPacket();
	int GetPacketNum() const
	{
		return mPacket;
	}
	void Run();
private:
	unsigned ReadByte(unsigned size);
	void ReadBuffer(unsigned size);
	void SkipByte(unsigned size);
	const char *GetTagName(unsigned tag_type) const;
	const char *GetFrameType(unsigned frame_type) const;
	const char *GetVideoCodecName(unsigned codec_id) const;
	const char *GetAudioCodecName(unsigned codec_id) const;
	IProtocol *mProtocol;
	unsigned mPacket;

	BitBuffer mBits;
	FlvHeader mHeader;
	std::vector<unsigned char> mBuffer;
};

#endif
