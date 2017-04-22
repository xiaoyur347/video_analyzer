#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "pes.h"

#include "debug.h"

void PES::PacketHeader::Analyze(BitBuffer &bits)
{
	packet_start_code_prefix = bits.GetByte(3);
	if (packet_start_code_prefix != 1)
	{
		LOG_ERROR("Error prefix %u", packet_start_code_prefix);
		return;
	}
	stream_id = bits.GetByte(1);

	PES_packet_length = bits.GetByte(2);

	marker_bits = bits.GetBit(2);
	PES_scrambling_control = bits.GetBit(2);
	PES_priority = bits.GetOneBit();
	data_alignment_indicator = bits.GetOneBit();
	copyright = bits.GetOneBit();
	original_or_copy = bits.GetOneBit();
	PTS_DTS_flags = bits.GetBit(2);
	ESCR_flag = bits.GetOneBit();
	ES_rate_flag = bits.GetOneBit();
	DSM_trick_mode_flag = bits.GetOneBit();
	additional_copy_info_flag = bits.GetOneBit();
	PES_CRC_flag = bits.GetOneBit();
	PES_extension_flag = bits.GetOneBit();

	PES_header_data_length = bits.GetByte(1);

	if (PTS_DTS_flags == 0x02)
	{
		//PTS only
		bits.GetBit(4); //0x02
		PTS = bits.GetBit(3);
		bits.GetBit(1);
		PTS = PTS << 15 | bits.GetBit(15);
		bits.GetBit(1);
		PTS = PTS << 15 | bits.GetBit(15);
		bits.GetBit(1);
	}
	else if (PTS_DTS_flags == 0x03)
	{
		//PTS + DTS
		bits.GetBit(4); //0x03
		PTS = bits.GetBit(3);
		bits.GetBit(1);
		PTS = PTS << 15 | bits.GetBit(15);
		bits.GetBit(1);
		PTS = PTS << 15 | bits.GetBit(15);
		bits.GetBit(1);

		bits.GetBit(4); //0x01
		DTS = bits.GetBit(3);
		bits.GetBit(1);
		DTS = DTS << 15 | bits.GetBit(15);
		bits.GetBit(1);
		DTS = DTS << 15 | bits.GetBit(15);
		bits.GetBit(1);
	}

	LOG_WARN("stream_id=%x,PTS_DTS_flags=%u,PTS=%" PRId64 ",DTS=%" PRId64,
		stream_id, PTS_DTS_flags, PTS, DTS);
}

void PES::Analyze(BitBuffer &bits, bool video)
{
	PacketHeader header;
	header.Analyze(bits);
	if (video)
	{

	}
	else
	{
		
	}
}
