#include "buffer.h"
#include <vector>

class TsFile
{
public:
	enum
	{
		TS_PACKET_SIZE = 188
	};
	struct PacketHeader
	{
		unsigned sync_byte:8;

		unsigned transport_error_indicator:1;
		unsigned payload_unit_start_indicator:1;
		unsigned transport_priority:1;
		unsigned pid:13;

		//H->L transport_scrambling_control|adaptation_field_control|continuity_counter
		unsigned transport_scrambling_control:2;
		unsigned adaptation_field_control:2;
		unsigned continuity_counter:4;

		const char *GetPidName() const;
		void Analyze(BitBuffer &bits);
		void Dump();
	};
	struct TS_PAT_Program
	{
		unsigned program_number:  16;  //节目号
		unsigned program_map_PID: 13;  // 节目映射表的PID，节目号大于0时对应的PID，每个节目对应一个
	};
	struct TS_PAT
	{
		unsigned table_id                     : 8;  //固定为0x00 ，标志是该表是PAT表
		unsigned section_syntax_indicator     : 1;  //段语法标志位，固定为1
		unsigned zero                         : 1;  //0
		unsigned reserved_1                   : 2;  // 保留位
		unsigned section_length               : 12; //表示从下一个字段开始到CRC32(含)之间有用的字节数
		unsigned transport_stream_id          : 16; //该传输流的ID，区别于一个网络中其它多路复用的流
		unsigned reserved_2                   : 2;  // 保留位
		unsigned version_number               : 5;  //范围0-31，表示PAT的版本号
		unsigned current_next_indicator       : 1;  //发送的PAT是当前有效还是下一个PAT有效
		unsigned section_number               : 8;  //分段的号码。PAT可能分为多段传输，第一段为00，以后每个分段加1，最多可能有256个分段
		unsigned last_section_number          : 8;  //最后一个分段的号码

		std::vector<TS_PAT_Program> program;
		unsigned reserved_3                   : 3;  // 保留位
		unsigned network_PID                  : 13; //网络信息表（NIT）的PID,节目号为0时对应的PID为network_PID
		unsigned CRC_32                       : 32; //CRC32校验码

		void Analyze(BitBuffer &bits);
		void Dump();
	};
	explicit TsFile(int fd);
	~TsFile();
	bool ReadPacket();
	bool AnalyzePacket();
	int GetPacketNum() const
	{
		return mPacket;
	}
private:
	int mFd;
	int mPacket;
	unsigned char mBuffer[TS_PACKET_SIZE];
	BitBuffer mBits;

	TS_PAT mPAT;
};
