#include "es.h"
#include "debug.h"
#include "type.h"

bool H264ES::NalHeader::Analyze(BitBuffer &bits)
{
	nalu_header = bits.GetByte(4);
	if (nalu_header != 0x00000001)
	{
		LOG_ERROR("Error nalu_header %u", nalu_header);
		return false;
	}
	forbidden_zero_bit = bits.GetOneBit();
	nal_ref_idc = bits.GetBit(2);
	nal_unit_type = bits.GetBit(5);

	LOG_INFO("nal_ref_idc=%u,nal_unit_type=%u %s", nal_ref_idc, nal_unit_type, GetName());
	return true;
}

const char *H264ES::NalHeader::GetName() const
{
	switch (nal_unit_type)
	{
		case 1:
			return "slice_layer_without_partitioning_rbsp";
		case 2:
			return "[I_Slice]slice_data_partition_a_layer_rbsp";
		case 3:
			return "[P_Slice]slice_data_partition_b_layer_rbsp";
		case 4:
			return "[B_Slice]slice_data_partition_c_layer_rbsp";
		case 5:
			return "[IDR]slice_layer_without_partitioning_rbsp";
		case 6:
			return "sei_rbsp";
		case 7:
			return "[SPS]seq_parameter_set_rbsp";
		case 8:
			return "[PPS]pic_parameter_set_rbsp";
		case 9:
			return "access_unit_delimiter_rbsp";
		case 10:
			return "end_of_seq_rbsp";
		case 11:
			return "end_of_stream_rbsp";
		case 12:
			return "filler_data_rbsp";
		case 13:
			return "seq_parameter_set_extension_rbsp";
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
			return "reserved";
		case 19:
			return "slice_layer_without_partitioning_rbsp";
		case 20:
		case 21:
		case 22:
		case 23:
			return "reserved";
		default:
			return "undefined";
	}
	return "undefined";
}

const char *H264ES::SPS::GetProfileName() const
{
	switch (profile_idc)
	{
		case FF_PROFILE_H264_HIGH:
			return "HIGH";
		case FF_PROFILE_H264_BASELINE:
			return "BASELINE";
		case FF_PROFILE_H264_MAIN:
			return "MAIN";
	}
	return "unknown";
}

std::string H264ES::SPS::GetLevelName() const
{
	char szBuffer[16] = {0};
	sprintf(szBuffer, "%u.%u", level_idc / 10, level_idc % 10);
	return szBuffer;
}

bool H264ES::SPS::Analyze(BitBuffer &bits)
{
	profile_idc = bits.GetByte(1);
	constraint_set0_flag = bits.GetOneBit();
	constraint_set1_flag = bits.GetOneBit();
	constraint_set2_flag = bits.GetOneBit();
	constraint_set3_flag = bits.GetOneBit();
	reserved_zero_4bits = bits.GetBit(4);
	level_idc = bits.GetByte(1);

	seq_parameter_set_id = bits.GetUEV();

	if (profile_idc == 100) //high
	{
		chroma_format_idc = bits.GetUEV();
		bit_depth_luma_minus8 = bits.GetUEV();
		bit_depth_chroma_minus8 = bits.GetUEV();
		qpprime_y_zero_transform_bypass_flag = bits.GetOneBit();
		seq_scaling_matrix_present_flag = bits.GetOneBit();
		LOG_WARN("chroma_format_idc=%u,bit_depth_luma_minus8=%u,bit_depth_chroma_minus8=%u",
			chroma_format_idc, bit_depth_luma_minus8, bit_depth_chroma_minus8);
	}

	log2_max_frame_num_minus4 = bits.GetUEV();
	pic_order_cnt_type = bits.GetUEV();

	if (pic_order_cnt_type == 0)
	{
		log2_max_pic_order_cnt_lsb_minus4 = bits.GetUEV();
	}
	else if (pic_order_cnt_type == 1)
	{
		delta_pic_order_always_zero_flag = bits.GetOneBit();
		offset_for_non_ref_pic = bits.GetSEV();
		offset_for_top_to_bottom_field = bits.GetSEV();
		num_ref_frames_in_pic_order_cnt_cycle = bits.GetUEV();
		for (unsigned i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++)
		{
			offset_for_ref_frame.push_back((signed char)bits.GetSEV());
		}
	}
	num_ref_frames = bits.GetUEV();
	gaps_in_frame_num_value_allowed_flag = bits.GetOneBit();
	pic_width_in_mbs_minus1 = bits.GetUEV();
	pic_height_in_map_units_minus1 = bits.GetUEV();
	frame_mbs_only_flag = bits.GetOneBit();
	if (!frame_mbs_only_flag)
	{
		mb_adaptive_frame_field_flag = bits.GetOneBit();
	}
	direct_8x8_inference_flag = bits.GetOneBit();
	frame_cropping_flag = bits.GetOneBit();
	if (frame_cropping_flag)
	{
		frame_crop_left_offset = bits.GetUEV();
		frame_crop_right_offset = bits.GetUEV();
		frame_crop_top_offset = bits.GetUEV();
		frame_crop_bottom_offset = bits.GetUEV();
	}
	vui_parameters_present_flag = bits.GetOneBit();

	LOG_ERROR("profile=%u(%s),level=%u(%s),num_ref_frames=%u,width=%u,height=%u",
		profile_idc, GetProfileName(), level_idc, GetLevelName().c_str(),
		num_ref_frames,
		(pic_width_in_mbs_minus1 + 1) * 16,
		(pic_height_in_map_units_minus1 + 1) * 16);
	return true;
}

const char *H264ES::Slice::GetSliceName() const
{
	switch (slice_type)
	{
		case 0:
			return "P";
		case 1:
			return "B";
		case 2:
			return "I";
		case 3:
			return "SP";
		case 4:
			return "SI";
		case 5:
			return "P";
		case 6:
			return "B";
		case 7:
			return "I";
		case 8:
			return "SP";
		case 9:
			return "SI";
		default:
			return "unknown";
	}
}

bool H264ES::Slice::Analyze(BitBuffer &bits)
{
	first_mb_in_slice = bits.GetUEV();
	slice_type = bits.GetUEV();
	pic_parameter_set_id = bits.GetUEV();
	LOG_ERROR("%u %s slice", slice_type, GetSliceName());
	return true;
}

void H264ES::JumpToNaluHeader(BitBuffer &bits)
{
	bits.SkipTrailing();
	while (!bits.IsEmpty())
	{
		unsigned temp = bits.GetByte(1);
		if (temp != 0)
		{
			continue;
		}
		temp = bits.GetByte(1);
		if (temp != 0)
		{
			continue;
		}
		temp = bits.GetByte(1);
		if (temp != 0)
		{
			continue;
		}
		temp = bits.GetByte(1);
		if (temp != 1)
		{
			continue;
		}
		bits.RewindBit(32);
		LOG_INFO("FIND NALU HEADER");
		return;
	}
}

bool H264ES::Analyze(BitBuffer &bits)
{
	NalHeader header;
	if (!header.Analyze(bits))
	{
		return false;
	}
	if (header.nal_unit_type == 9)
	{
		bits.SkipByte(1);
		return true;
	}
	else if (header.nal_unit_type == 7)
	{
		//SPS
		SPS sps;
		sps.Analyze(bits);
		JumpToNaluHeader(bits);
	}
	else if (header.nal_unit_type == 8)
	{
		//PPS
		JumpToNaluHeader(bits);
	}
	else if (header.nal_unit_type == 6)
	{
		//SEI
		JumpToNaluHeader(bits);
	}
	else if (header.nal_unit_type == 5)
	{
		//IDR
		Slice slice;
		slice.Analyze(bits);
		JumpToNaluHeader(bits);
	}
	else if (header.nal_unit_type == 1)
	{
		Slice slice;
		slice.Analyze(bits);
		JumpToNaluHeader(bits);
	}
	else if (header.nal_unit_type == 2)
	{
		Slice slice;
		slice.Analyze(bits);
		JumpToNaluHeader(bits);
	}
	else if (header.nal_unit_type == 3)
	{
		Slice slice;
		slice.Analyze(bits);
		JumpToNaluHeader(bits);
	}
	else if (header.nal_unit_type == 4)
	{
		Slice slice;
		slice.Analyze(bits);
		JumpToNaluHeader(bits);
	}
	return true;
}

bool MPEGAudioES::PacketHeader::Analyze(BitBuffer &bits)
{
	sync_word = bits.GetBit(12);
	if (sync_word != 0xfff)
	{
		LOG_ERROR("Error sync_word %u", sync_word);
		return false;
	}
	ID = bits.GetOneBit();
	layer = bits.GetBit(2);

	no_protection = bits.GetOneBit();
	bit_rate_index = bits.GetBit(4);
	sampling_frequency = bits.GetBit(2);
	padding = bits.GetOneBit();
	private_bit = bits.GetOneBit();
	mode = bits.GetBit(2);
	mode_extension = bits.GetBit(2);
	copyright = bits.GetOneBit();
	original_or_copy = bits.GetOneBit();
	emphasis = bits.GetBit(2);

	LOG_WARN("ID=%s,layer=%u,sampling_frequency=%s kHz",
		ID == 1 ? "mpeg1" : "mpeg2",
		4 - layer,
	 	sampling_frequency == 0 ? "44.1" : sampling_frequency == 1 ? "48" : "32");
	return true;
}

bool MPEGAudioES::Analyze(BitBuffer &bits)
{
	PacketHeader header;
	if (!header.Analyze(bits))
	{
		return false;
	}
	return true;
}

const char *AACES::ADTSHeader::GetSampleRate() const
{
	const char *table[] = {
		"96000",
		"88200",
		"64000",
		"48000",
		"44100",
		"32000",
		"24000",
		"22050",
		"16000",
		"12000",
		"11025",
		"8000",
		"7350"
	};
	return table[sampling_frequency_index];
}

bool AACES::ADTSHeader::Analyze(BitBuffer &bits)
{
	sync_word = bits.GetBit(12);
	if (sync_word != 0xfff)
	{
		LOG_ERROR("Error sync_word %u", sync_word);
		return false;
	}
	ID = bits.GetOneBit();
	layer = bits.GetBit(2);

	no_protection = bits.GetOneBit();

	profile = bits.GetBit(2);
	sampling_frequency_index = bits.GetBit(4);
	private_bit = bits.GetOneBit();
	channel_configuration = bits.GetBit(3);
	original_or_copy = bits.GetOneBit();
	home = bits.GetOneBit();

	LOG_WARN("ID=%s,layer=%u,profile=%u,sampling_frequency_index=%s Hz",
		ID == 1 ? "mpeg2" : "mpeg4",
		layer,
		profile,
	 	GetSampleRate());
	return true;
}

bool AACES::Analyze(BitBuffer &bits)
{
	ADTSHeader header;
	if (!header.Analyze(bits))
	{
		return false;
	}
	return true;
}
