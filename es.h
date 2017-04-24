#ifndef ES_H
#define ES_H

#include "buffer.h"
#include <vector>
#include <string>

class H264ES
{
public:
	struct NalHeader
	{
		unsigned nalu_header : 32;
		unsigned forbidden_zero_bit : 1;
		unsigned nal_ref_idc : 2;
		unsigned nal_unit_type : 5;

		NalHeader()
			:nalu_header(0),
			forbidden_zero_bit(0),
			nal_ref_idc(0),
			nal_unit_type(0)
		{
		}
		bool Analyze(BitBuffer &bits);
		const char *GetName() const;
	};
	struct VUI
	{
		unsigned aspect_ratio_info_present_flag : 1;
		unsigned aspect_ratio_idc : 8;
		unsigned sar_width : 16;
		unsigned sar_height : 16;

		unsigned overscan_info_present_flag : 1;
		//unsigned
	};
	struct SPS //H264 7.3.2.1
	{
		unsigned profile_idc : 8;
		unsigned constraint_set0_flag : 1;
		unsigned constraint_set1_flag : 1;
		unsigned constraint_set2_flag : 1;
		unsigned constraint_set3_flag : 1;
		unsigned reserved_zero_4bits : 4;
		unsigned level_idc : 8;

		//and more
		unsigned seq_parameter_set_id;

		//only in high
		unsigned chroma_format_idc;
		unsigned bit_depth_luma_minus8;
		unsigned bit_depth_chroma_minus8;
		unsigned qpprime_y_zero_transform_bypass_flag : 1;
		unsigned seq_scaling_matrix_present_flag : 1;

		unsigned log2_max_frame_num_minus4;
		unsigned pic_order_cnt_type;
		unsigned log2_max_pic_order_cnt_lsb_minus4;

		unsigned delta_pic_order_always_zero_flag : 1;
		int offset_for_non_ref_pic;
		int offset_for_top_to_bottom_field;
		unsigned num_ref_frames_in_pic_order_cnt_cycle;
		std::vector<signed char> offset_for_ref_frame;

		unsigned num_ref_frames;
		unsigned gaps_in_frame_num_value_allowed_flag : 1;
		unsigned pic_width_in_mbs_minus1;
		unsigned pic_height_in_map_units_minus1;
		unsigned frame_mbs_only_flag : 1;
		unsigned mb_adaptive_frame_field_flag : 1;
		unsigned direct_8x8_inference_flag : 1;
		unsigned frame_cropping_flag : 1;

		unsigned frame_crop_left_offset;
		unsigned frame_crop_right_offset;
		unsigned frame_crop_top_offset;
		unsigned frame_crop_bottom_offset;

		unsigned vui_parameters_present_flag : 1;

		SPS()
			:profile_idc(0),
			constraint_set0_flag(0),
			constraint_set1_flag(0),
			constraint_set2_flag(0),
			constraint_set3_flag(0),
			reserved_zero_4bits(0),
			level_idc(0),
			seq_parameter_set_id(0),
			chroma_format_idc(0),
			bit_depth_luma_minus8(0),
			bit_depth_chroma_minus8(0),
			qpprime_y_zero_transform_bypass_flag(0),
			seq_scaling_matrix_present_flag(0),
			log2_max_frame_num_minus4(0),
			pic_order_cnt_type(0),
			log2_max_pic_order_cnt_lsb_minus4(0),
			delta_pic_order_always_zero_flag(0),
			offset_for_non_ref_pic(0),
			offset_for_top_to_bottom_field(0),
			num_ref_frames_in_pic_order_cnt_cycle(0),
			num_ref_frames(0),
			gaps_in_frame_num_value_allowed_flag(0),
			pic_width_in_mbs_minus1(0),
			pic_height_in_map_units_minus1(0),
			frame_mbs_only_flag(0),
			mb_adaptive_frame_field_flag(0),
			direct_8x8_inference_flag(0),
			frame_cropping_flag(0),
			frame_crop_left_offset(0),
			frame_crop_right_offset(0),
			frame_crop_top_offset(0),
			frame_crop_bottom_offset(0),
			vui_parameters_present_flag(0)
		{

		}
		bool Analyze(BitBuffer &bits);
		const char *GetProfileName() const;
		std::string GetLevelName() const;
	};
	struct Slice
	{
		unsigned first_mb_in_slice;
		unsigned slice_type;

		Slice()
			:first_mb_in_slice(0),
			slice_type(0)
		{

		}
		bool Analyze(BitBuffer &bits);
		const char *GetSliceName() const;
	};
	bool Analyze(BitBuffer &bits);
	void JumpToNaluHeader(BitBuffer &bits);
};

class MPEGAudioES
{
public:
	struct PacketHeader
	{
		unsigned sync_word : 12; //0xfff
		unsigned ID : 1; //'1'=mpeg1 '0'=mpeg2
		unsigned layer : 2; //'11'=1 '10'=2 '01'=3
		unsigned no_protection : 1;
		unsigned bit_rate_index : 4;
		unsigned sampling_frequency : 2; //kHz '00'=44.1 '01'=48 '10'=32
		unsigned padding : 1;
		unsigned private_bit : 1;
		unsigned mode : 2; //'00'=Stereo '01'=joint stereo '10'=dual channel '11'=single channel
		unsigned mode_extension : 2;
		unsigned copyright : 1; //0=none 1=yes
		unsigned original_or_copy : 1; //0=copy 1=original
		unsigned emphasis : 2;

		PacketHeader()
			:sync_word(0),
			 ID(0),
			 layer(0),
			 no_protection(0),
			 bit_rate_index(0),
			 sampling_frequency(0),
			 padding(0),
			 private_bit(0),
			 mode(0),
			 mode_extension(0),
			 copyright(0),
			 original_or_copy(0),
			 emphasis(0)
		{

		}
		bool Analyze(BitBuffer &bits);
	};

	bool Analyze(BitBuffer &bits);
};

class AACES
{
public:
	struct ADTSHeader
	{
		unsigned sync_word : 24;
		//2 bytes
		unsigned ID : 1;
		unsigned layer : 2;
		unsigned no_protection : 1;
		unsigned profile : 2;
		unsigned sampling_frequency_index : 4;
		unsigned private_bit : 1;
		unsigned channel_configuration : 3;
		unsigned original_or_copy : 1;
		unsigned home : 1;

		unsigned copyright_identification_bit : 1;
		unsigned copyright_identification_start : 1;
		unsigned aac_frame_length : 13;
		unsigned adts_buffer_fullness : 11;
		unsigned number_of_raw_data_blocks_in_frame : 2;
		ADTSHeader()
			:sync_word(0),
			ID(0),
			layer(0),
			no_protection(0),
			profile(0),
			sampling_frequency_index(0),
			private_bit(0),
			channel_configuration(0),
			original_or_copy(0),
			home(0),
			copyright_identification_bit(0),
			copyright_identification_start(0),
			aac_frame_length(0),
			adts_buffer_fullness(0),
			number_of_raw_data_blocks_in_frame(0)
		{

		}
		bool Analyze(BitBuffer &bits);
		const char *GetSampleRate() const;
	};
	bool Analyze(BitBuffer &bits);
};

#endif
