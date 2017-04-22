#ifndef PES_H
#define PES_H

#include "buffer.h"

class PES
{
public:
	struct PacketHeader
	{
		unsigned packet_start_code_prefix : 24;
		unsigned stream_id : 8;
		unsigned PES_packet_length : 16;
		//optional
		unsigned marker_bits : 2; //0x02
		unsigned PES_scrambling_control : 2;
		unsigned PES_priority : 1;
		unsigned data_alignment_indicator : 1;
		unsigned copyright : 1;
		unsigned original_or_copy : 1;
		unsigned PTS_DTS_flags : 2; // 3 both, 2 pts, 0 none
		unsigned ESCR_flag : 1;
		unsigned ES_rate_flag : 1;
		unsigned DSM_trick_mode_flag : 1;
		unsigned additional_copy_info_flag : 1;
		unsigned PES_CRC_flag : 1;
		unsigned PES_extension_flag : 1;
		unsigned PES_header_data_length : 8;

		PacketHeader()
			:packet_start_code_prefix(0),
			 stream_id(0),
			 PES_packet_length(0),
			 marker_bits(0),
			 PES_scrambling_control(0),
			 PES_priority(0),
			 data_alignment_indicator(0),
			 copyright(0),
			 original_or_copy(0),
			 PTS_DTS_flags(0),
			 ESCR_flag(0),
			 ES_rate_flag(0),
			 DSM_trick_mode_flag(0),
			 additional_copy_info_flag(0),
			 PES_CRC_flag(0),
			 PES_extension_flag(0),
			 PES_header_data_length(0)
		{

		}
		void Analyze(BitBuffer &bits);
	};

	void Analyze(BitBuffer &bits);
};

#endif
