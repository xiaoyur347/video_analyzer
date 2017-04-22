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

	LOG_INFO("stream_id=%x,PTS_DTS_flags=%u",
		stream_id, PTS_DTS_flags);
}

void PES::Analyze(BitBuffer &bits)
{
	PacketHeader header;
	header.Analyze(bits);
}
