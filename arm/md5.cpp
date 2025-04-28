#include "md5.h"
#include <iomanip>
#include <assert.h>
#include <chrono>
#include<arm_neon.h>

using namespace std;
using namespace chrono;

/**
*StringProcess: 将单个输入字符串转换成MD5计算所需的消息数组
*@param input 输入
*@param[out] n_byte 用于给调用者传递额外的返回值，即最终Byte数组的长度
*@return Byte消息数组
 */
Byte *StringProcess(string input,int *n_byte)
{
	// 将输入的字符串转换为Byte为单位的数组
	Byte *blocks=(Byte *)input.c_str();
	int length=input.length();

	// 计算原始消息长度（以比特为单位）
	int bitLength=length*8;

	// paddingBits: 原始消息需要的padding长度（以bit为单位）
	// 对于给定的消息，将其补齐至length%512==448为止
	// 需要注意的是，即便给定的消息满足length%512==448，也需要再pad 512bits
	int paddingBits=bitLength % 512;
	if (paddingBits > 448)
	{
		paddingBits=512 - (paddingBits - 448);
	}
	else if (paddingBits<448)
	{
		paddingBits=448 - paddingBits;
	}
	else if (paddingBits == 448)
	{
		paddingBits=512;
	}

	// 原始消息需要的padding长度（以Byte为单位）
	int paddingBytes=paddingBits / 8;
	// 创建最终的字节数组
	// length+paddingBytes+8:
	// 1. length为原始消息的长度（bits）
	// 2. paddingBytes为原始消息需要的padding长度（Bytes）
	// 3. 在pad到length%512==448之后，需要额外附加64bits的原始消息长度，即8个bytes
	int paddedLength=length+paddingBytes+8;
	Byte *paddedMessage=new Byte[paddedLength];

	// 复制原始消息
	memcpy(paddedMessage,blocks,length);

	// 添加填充字节。填充时，第一位为1，后面的所有位均为0。
	// 所以第一个byte是0x80
	paddedMessage[length]=0x80;							 // 添加一个0x80字节
	memset(paddedMessage+length+1,0,paddingBytes - 1);// 填充0字节

	// 添加消息长度（64比特，小端格式）
	for (int i=0;i<8;++i)
	{
		// 特别注意此处应当将bitLength转换为uint64_t
		// 这里的length是原始消息的长度
		paddedMessage[length+paddingBytes+i]=((uint64_t)length*8 >> (i*8)) & 0xFF;
	}

	// 验证长度是否满足要求。此时长度应当是512bit的倍数
	int residual=8*paddedLength % 512;
	// assert(residual == 0);

	// 在填充+添加长度之后，消息被分为n_blocks个512bit的部分
	*n_byte=paddedLength;
	return paddedMessage;
}


/**
*MD5Hash: 将单个输入字符串转换成MD5
*@param input 输入
*@param[out] state 用于给调用者传递额外的返回值，即最终的缓冲区，也就是MD5的结果
*@return Byte消息数组
 */
void MD5Hash(string input,bit32 *state)
{

	Byte *paddedMessage;
	int *messageLength=new int[1];
	for (int i=0;i<1;i+=1)
	{
		paddedMessage=StringProcess(input,&messageLength[i]);
		// cout<<messageLength[i]<<endl;
		assert(messageLength[i] == messageLength[0]);
	}
	int n_blocks=messageLength[0] / 64;

	// bit32* state= new bit32[4];
	state[0]=0x67452301;
	state[1]=0xefcdab89;
	state[2]=0x98badcfe;
	state[3]=0x10325476;

	// 逐block地更新state
	for (int i=0;i<n_blocks;i+=1)
	{
		bit32 x[16];

		// 下面的处理，在理解上较为复杂
		for (int i1=0;i1<16;++i1)
		{
			x[i1]=(paddedMessage[4*i1+i*64]) |
					(paddedMessage[4*i1+1+i*64] << 8) |
					(paddedMessage[4*i1+2+i*64] << 16) |
					(paddedMessage[4*i1+3+i*64] << 24);
		}

		bit32 a=state[0],b=state[1],c=state[2],d=state[3];

		auto start=system_clock::now();
		/* Round 1 */
		FF(a,b,c,d,x[0],s11,0xd76aa478);
		FF(d,a,b,c,x[1],s12,0xe8c7b756);
		FF(c,d,a,b,x[2],s13,0x242070db);
		FF(b,c,d,a,x[3],s14,0xc1bdceee);
		FF(a,b,c,d,x[4],s11,0xf57c0faf);
		FF(d,a,b,c,x[5],s12,0x4787c62a);
		FF(c,d,a,b,x[6],s13,0xa8304613);
		FF(b,c,d,a,x[7],s14,0xfd469501);
		FF(a,b,c,d,x[8],s11,0x698098d8);
		FF(d,a,b,c,x[9],s12,0x8b44f7af);
		FF(c,d,a,b,x[10],s13,0xffff5bb1);
		FF(b,c,d,a,x[11],s14,0x895cd7be);
		FF(a,b,c,d,x[12],s11,0x6b901122);
		FF(d,a,b,c,x[13],s12,0xfd987193);
		FF(c,d,a,b,x[14],s13,0xa679438e);
		FF(b,c,d,a,x[15],s14,0x49b40821);

		/* Round 2 */
		GG(a,b,c,d,x[1],s21,0xf61e2562);
		GG(d,a,b,c,x[6],s22,0xc040b340);
		GG(c,d,a,b,x[11],s23,0x265e5a51);
		GG(b,c,d,a,x[0],s24,0xe9b6c7aa);
		GG(a,b,c,d,x[5],s21,0xd62f105d);
		GG(d,a,b,c,x[10],s22,0x2441453);
		GG(c,d,a,b,x[15],s23,0xd8a1e681);
		GG(b,c,d,a,x[4],s24,0xe7d3fbc8);
		GG(a,b,c,d,x[9],s21,0x21e1cde6);
		GG(d,a,b,c,x[14],s22,0xc33707d6);
		GG(c,d,a,b,x[3],s23,0xf4d50d87);
		GG(b,c,d,a,x[8],s24,0x455a14ed);
		GG(a,b,c,d,x[13],s21,0xa9e3e905);
		GG(d,a,b,c,x[2],s22,0xfcefa3f8);
		GG(c,d,a,b,x[7],s23,0x676f02d9);
		GG(b,c,d,a,x[12],s24,0x8d2a4c8a);

		/* Round 3 */
		HH(a,b,c,d,x[5],s31,0xfffa3942);
		HH(d,a,b,c,x[8],s32,0x8771f681);
		HH(c,d,a,b,x[11],s33,0x6d9d6122);
		HH(b,c,d,a,x[14],s34,0xfde5380c);
		HH(a,b,c,d,x[1],s31,0xa4beea44);
		HH(d,a,b,c,x[4],s32,0x4bdecfa9);
		HH(c,d,a,b,x[7],s33,0xf6bb4b60);
		HH(b,c,d,a,x[10],s34,0xbebfbc70);
		HH(a,b,c,d,x[13],s31,0x289b7ec6);
		HH(d,a,b,c,x[0],s32,0xeaa127fa);
		HH(c,d,a,b,x[3],s33,0xd4ef3085);
		HH(b,c,d,a,x[6],s34,0x4881d05);
		HH(a,b,c,d,x[9],s31,0xd9d4d039);
		HH(d,a,b,c,x[12],s32,0xe6db99e5);
		HH(c,d,a,b,x[15],s33,0x1fa27cf8);
		HH(b,c,d,a,x[2],s34,0xc4ac5665);

		/* Round 4 */
		II(a,b,c,d,x[0],s41,0xf4292244);
		II(d,a,b,c,x[7],s42,0x432aff97);
		II(c,d,a,b,x[14],s43,0xab9423a7);
		II(b,c,d,a,x[5],s44,0xfc93a039);
		II(a,b,c,d,x[12],s41,0x655b59c3);
		II(d,a,b,c,x[3],s42,0x8f0ccc92);
		II(c,d,a,b,x[10],s43,0xffeff47d);
		II(b,c,d,a,x[1],s44,0x85845dd1);
		II(a,b,c,d,x[8],s41,0x6fa87e4f);
		II(d,a,b,c,x[15],s42,0xfe2ce6e0);
		II(c,d,a,b,x[6],s43,0xa3014314);
		II(b,c,d,a,x[13],s44,0x4e0811a1);
		II(a,b,c,d,x[4],s41,0xf7537e82);
		II(d,a,b,c,x[11],s42,0xbd3af235);
		II(c,d,a,b,x[2],s43,0x2ad7d2bb);
		II(b,c,d,a,x[9],s44,0xeb86d391);

		state[0]+=a;
		state[1]+=b;
		state[2]+=c;
		state[3]+=d;
	}

	// 下面的处理，在理解上较为复杂
	for (int i=0;i<4;i++)
	{
		uint32_t value=state[i];
		state[i]=((value & 0xff) << 24) |		 // 将最低字节移到最高位
				   ((value & 0xff00) << 8) |	 // 将次低字节左移
				   ((value & 0xff0000) >> 8) |	 // 将次高字节右移
				   ((value & 0xff000000) >> 24);// 将最高字节移到最低位
	}

	// 输出最终的hash结果
	// for (int i1=0;i1<4;i1+=1)
	// {
	// 	cout << std::setw(8) << std::setfill('0') << hex << state[i1];
	// }
	// cout << endl;

	// 释放动态分配的内存
	// 实现SIMD并行算法的时候，也请记得及时回收内存！
	delete[] paddedMessage;
	delete[] messageLength;
}
void MD5Hash_simd(const string inputs[4],bit32 states[4][4]){
	Byte *paddedMessages[4];
	int messageLengths[4];
	for(int i=0;i<4;i++){
        paddedMessages[i]=StringProcess(inputs[i],&messageLengths[i]);
    }
	int max=messageLengths[0];
	for(int i=0;i<4;i++){
		if(messageLengths[i]>max){
			max=messageLengths[i];
		}
	}
	for(int i=0;i<4;i++){
		if(messageLengths[i]<max){
			Byte* newmessage=new Byte[max];
			memcpy(newmessage,paddedMessages[i],messageLengths[i]);
			memset(newmessage+messageLengths[i],0,max-messageLengths[i]);
			delete[] paddedMessages[i];
			paddedMessages[i]=newmessage;
			messageLengths[i]=max;
		}
	}
	int n_blocks=max/64;
	uint32x4_t a,b,c,d;
	a=(uint32x4_t){0x67452301,0x67452301,0x67452301,0x67452301};
    b=(uint32x4_t){0xefcdab89,0xefcdab89,0xefcdab89,0xefcdab89};
    c=(uint32x4_t){0x98badcfe,0x98badcfe,0x98badcfe,0x98badcfe};
    d=(uint32x4_t){0x10325476,0x10325476,0x10325476,0x10325476};
    static const uint32_t ac_constants[64]={
        0xd76aa478,0xe8c7b756,0x242070db,0xc1bdceee,
        0xf57c0faf,0x4787c62a,0xa8304613,0xfd469501,
        0x698098d8,0x8b44f7af,0xffff5bb1,0x895cd7be,
        0x6b901122,0xfd987193,0xa679438e,0x49b40821,
        0xf61e2562,0xc040b340,0x265e5a51,0xe9b6c7aa,
        0xd62f105d,0x02441453,0xd8a1e681,0xe7d3fbc8,
        0x21e1cde6,0xc33707d6,0xf4d50d87,0x455a14ed,
        0xa9e3e905,0xfcefa3f8,0x676f02d9,0x8d2a4c8a,
        0xfffa3942,0x8771f681,0x6d9d6122,0xfde5380c,
        0xa4beea44,0x4bdecfa9,0xf6bb4b60,0xbebfbc70,
        0x289b7ec6,0xeaa127fa,0xd4ef3085,0x04881d05,
        0xd9d4d039,0xe6db99e5,0x1fa27cf8,0xc4ac5665,
        0xf4292244,0x432aff97,0xab9423a7,0xfc93a039,
        0x655b59c3,0x8f0ccc92,0xffeff47d,0x85845dd1,
        0x6fa87e4f,0xfe2ce6e0,0xa3014314,0x4e0811a1,
        0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391
    };
	uint32x4_t ac_vec[64];
	for (int i=0;i<64;i++) {
        ac_vec[i]=vdupq_n_u32(ac_constants[i]);
    }	
	for(int block=0;block<n_blocks;block++){
		uint32x4_t x_vec[16];
		for (int i=0;i<16;++i) {
            x_vec[i]=(uint32x4_t){
                get(paddedMessages[0],block,i),
                get(paddedMessages[1],block,i),
                get(paddedMessages[2],block,i),
                get(paddedMessages[3],block,i)
            };
        }
	    uint32x4_t aa=a,bb=b,cc=c,dd=d;
	    neon_FF(&a,b,c,d,x_vec[0],s11,ac_vec[0]);
        neon_FF(&d,a,b,c,x_vec[1],s12,ac_vec[1]);
        neon_FF(&c,d,a,b,x_vec[2],s13,ac_vec[2]);
        neon_FF(&b,c,d,a,x_vec[3],s14,ac_vec[3]);
        neon_FF(&a,b,c,d,x_vec[4],s11,ac_vec[4]);
        neon_FF(&d,a,b,c,x_vec[5],s12,ac_vec[5]);
        neon_FF(&c,d,a,b,x_vec[6],s13,ac_vec[6]);
        neon_FF(&b,c,d,a,x_vec[7],s14,ac_vec[7]);
        neon_FF(&a,b,c,d,x_vec[8],s11,ac_vec[8]);
        neon_FF(&d,a,b,c,x_vec[9],s12,ac_vec[9]);
        neon_FF(&c,d,a,b,x_vec[10],s13,ac_vec[10]);
        neon_FF(&b,c,d,a,x_vec[11],s14,ac_vec[11]);
        neon_FF(&a,b,c,d,x_vec[12],s11,ac_vec[12]);
        neon_FF(&d,a,b,c,x_vec[13],s12,ac_vec[13]);
        neon_FF(&c,d,a,b,x_vec[14],s13,ac_vec[14]);
        neon_FF(&b,c,d,a,x_vec[15],s14,ac_vec[15]);

        /* Round 2 */
        neon_GG(&a,b,c,d,x_vec[1],s21,ac_vec[16]);
        neon_GG(&d,a,b,c,x_vec[6],s22,ac_vec[17]);
        neon_GG(&c,d,a,b,x_vec[11],s23,ac_vec[18]);
        neon_GG(&b,c,d,a,x_vec[0],s24,ac_vec[19]);
        neon_GG(&a,b,c,d,x_vec[5],s21,ac_vec[20]);
        neon_GG(&d,a,b,c,x_vec[10],s22,ac_vec[21]);
        neon_GG(&c,d,a,b,x_vec[15],s23,ac_vec[22]);
        neon_GG(&b,c,d,a,x_vec[4],s24,ac_vec[23]);
        neon_GG(&a,b,c,d,x_vec[9],s21,ac_vec[24]);
        neon_GG(&d,a,b,c,x_vec[14],s22,ac_vec[25]);
        neon_GG(&c,d,a,b,x_vec[3],s23,ac_vec[26]);
        neon_GG(&b,c,d,a,x_vec[8],s24,ac_vec[27]);
        neon_GG(&a,b,c,d,x_vec[13],s21,ac_vec[28]);
        neon_GG(&d,a,b,c,x_vec[2],s22,ac_vec[29]);
        neon_GG(&c,d,a,b,x_vec[7],s23,ac_vec[30]);
        neon_GG(&b,c,d,a,x_vec[12],s24,ac_vec[31]);

        /* Round 3 */
        neon_HH(&a,b,c,d,x_vec[5],s31,ac_vec[32]);
        neon_HH(&d,a,b,c,x_vec[8],s32,ac_vec[33]);
        neon_HH(&c,d,a,b,x_vec[11],s33,ac_vec[34]);
        neon_HH(&b,c,d,a,x_vec[14],s34,ac_vec[35]);
        neon_HH(&a,b,c,d,x_vec[1],s31,ac_vec[36]);
        neon_HH(&d,a,b,c,x_vec[4],s32,ac_vec[37]);
        neon_HH(&c,d,a,b,x_vec[7],s33,ac_vec[38]);
        neon_HH(&b,c,d,a,x_vec[10],s34,ac_vec[39]);
        neon_HH(&a,b,c,d,x_vec[13],s31,ac_vec[40]);
        neon_HH(&d,a,b,c,x_vec[0],s32,ac_vec[41]);
        neon_HH(&c,d,a,b,x_vec[3],s33,ac_vec[42]);
        neon_HH(&b,c,d,a,x_vec[6],s34,ac_vec[43]);
        neon_HH(&a,b,c,d,x_vec[9],s31,ac_vec[44]);
        neon_HH(&d,a,b,c,x_vec[12],s32,ac_vec[45]);
        neon_HH(&c,d,a,b,x_vec[15],s33,ac_vec[46]);
        neon_HH(&b,c,d,a,x_vec[2],s34,ac_vec[47]);

        /* Round 4 */
        neon_II(&a,b,c,d,x_vec[0],s41,ac_vec[48]);
        neon_II(&d,a,b,c,x_vec[7],s42,ac_vec[49]);
        neon_II(&c,d,a,b,x_vec[14],s43,ac_vec[50]);
        neon_II(&b,c,d,a,x_vec[5],s44,ac_vec[51]);
        neon_II(&a,b,c,d,x_vec[12],s41,ac_vec[52]);
        neon_II(&d,a,b,c,x_vec[3],s42,ac_vec[53]);
        neon_II(&c,d,a,b,x_vec[10],s43,ac_vec[54]);
        neon_II(&b,c,d,a,x_vec[1],s44,ac_vec[55]);
        neon_II(&a,b,c,d,x_vec[8],s41,ac_vec[56]);
        neon_II(&d,a,b,c,x_vec[15],s42,ac_vec[57]);
        neon_II(&c,d,a,b,x_vec[6],s43,ac_vec[58]);
        neon_II(&b,c,d,a,x_vec[13],s44,ac_vec[59]);
        neon_II(&a,b,c,d,x_vec[4],s41,ac_vec[60]);
        neon_II(&d,a,b,c,x_vec[11],s42,ac_vec[61]);
        neon_II(&c,d,a,b,x_vec[2],s43,ac_vec[62]);
        neon_II(&b,c,d,a,x_vec[9],s44,ac_vec[63]);

		a=vaddq_u32(a,aa);
        b=vaddq_u32(b,bb);
        c=vaddq_u32(c,cc);
        d=vaddq_u32(d,dd);
	}
    // 下面的处理，在理解上较为复杂
    states[0][0]=byte_swap(vgetq_lane_u32(a,0));
    states[1][0]=byte_swap(vgetq_lane_u32(a,1));
    states[2][0]=byte_swap(vgetq_lane_u32(a,2));
    states[3][0]=byte_swap(vgetq_lane_u32(a,3));

    states[0][1]=byte_swap(vgetq_lane_u32(b,0));
    states[1][1]=byte_swap(vgetq_lane_u32(b,1));
    states[2][1]=byte_swap(vgetq_lane_u32(b,2));
    states[3][1]=byte_swap(vgetq_lane_u32(b,3));

    states[0][2]=byte_swap(vgetq_lane_u32(c,0));
    states[1][2]=byte_swap(vgetq_lane_u32(c,1));
    states[2][2]=byte_swap(vgetq_lane_u32(c,2));
    states[3][2]=byte_swap(vgetq_lane_u32(c,3));

    states[0][3]=byte_swap(vgetq_lane_u32(d,0));
    states[1][3]=byte_swap(vgetq_lane_u32(d,1));
    states[2][3]=byte_swap(vgetq_lane_u32(d,2));
    states[3][3]=byte_swap(vgetq_lane_u32(d,3));

    // 释放动态分配的内存
    // 实现SIMD并行算法的时候，也请记得及时回收内存！
    for (int i=0;i<4;i++){
        delete[] paddedMessages[i];
    }
}

void MD5Hash_simd8(const string inputs[8],bit32 states[8][4]) {
		Byte* paddedMessages[8];
		int messageLengths[8];
		for (int i=0;i<8;i++) {
			paddedMessages[i]=StringProcess(inputs[i],&messageLengths[i]);
		}
		int maxLength=messageLengths[0];
		for (int i=1;i<8;++i)
			if (messageLengths[i] > maxLength) maxLength=messageLengths[i];
		for (int i=0;i<8;++i) {
			if (messageLengths[i]<maxLength) {
				Byte* newBuf=new Byte[maxLength];
				memcpy(newBuf,paddedMessages[i],messageLengths[i]);
				memset(newBuf + messageLengths[i],0,maxLength - messageLengths[i]);
				delete[] paddedMessages[i];
				paddedMessages[i]=newBuf;
				messageLengths[i]=maxLength;
			}
		}
		int n_blocks=maxLength / 64;
		uint32x4_t a0={0x67452301,0x67452301,0x67452301,0x67452301};
		uint32x4_t b0={0xefcdab89,0xefcdab89,0xefcdab89,0xefcdab89};
		uint32x4_t c0={0x98badcfe,0x98badcfe,0x98badcfe,0x98badcfe};
		uint32x4_t d0={0x10325476,0x10325476,0x10325476,0x10325476};
	
		uint32x4_t a1=a0,b1=b0,c1=c0,d1=d0;
		static const uint32_t ac_constants[64]={
			0xd76aa478,0xe8c7b756,0x242070db,0xc1bdceee,
			0xf57c0faf,0x4787c62a,0xa8304613,0xfd469501,
			0x698098d8,0x8b44f7af,0xffff5bb1,0x895cd7be,
			0x6b901122,0xfd987193,0xa679438e,0x49b40821,
			0xf61e2562,0xc040b340,0x265e5a51,0xe9b6c7aa,
			0xd62f105d,0x02441453,0xd8a1e681,0xe7d3fbc8,
			0x21e1cde6,0xc33707d6,0xf4d50d87,0x455a14ed,
			0xa9e3e905,0xfcefa3f8,0x676f02d9,0x8d2a4c8a,
			0xfffa3942,0x8771f681,0x6d9d6122,0xfde5380c,
			0xa4beea44,0x4bdecfa9,0xf6bb4b60,0xbebfbc70,
			0x289b7ec6,0xeaa127fa,0xd4ef3085,0x04881d05,
			0xd9d4d039,0xe6db99e5,0x1fa27cf8,0xc4ac5665,
			0xf4292244,0x432aff97,0xab9423a7,0xfc93a039,
			0x655b59c3,0x8f0ccc92,0xffeff47d,0x85845dd1,
			0x6fa87e4f,0xfe2ce6e0,0xa3014314,0x4e0811a1,
			0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391
		};
		uint32x4_t ac_vec[64];
		for (int i=0;i<64;i++)
			ac_vec[i]=vdupq_n_u32(ac_constants[i]);
		for (int block=0;block<n_blocks;++block) {
			uint32x4_t x_vec0[16],x_vec1[16];
			for (int i=0;i<16;i++) {
				x_vec0[i]=(uint32x4_t){
					get(paddedMessages[0],block,i),
					get(paddedMessages[1],block,i),
					get(paddedMessages[2],block,i),
					get(paddedMessages[3],block,i)
				};
				x_vec1[i]=(uint32x4_t){
					get(paddedMessages[4],block,i),
					get(paddedMessages[5],block,i),
					get(paddedMessages[6],block,i),
					get(paddedMessages[7],block,i)
				};
			}
			uint32x4_t aa0=a0,bb0=b0,cc0=c0,dd0=d0;
			uint32x4_t aa1=a1,bb1=b1,cc1=c1,dd1=d1;
		neon_FF(&a0,b0,c0,d0,x_vec0[0],s11,ac_vec[0]);
        neon_FF(&d0,a0,b0,c0,x_vec0[1],s12,ac_vec[1]);
        neon_FF(&c0,d0,a0,b0,x_vec0[2],s13,ac_vec[2]);
        neon_FF(&b0,c0,d0,a0,x_vec0[3],s14,ac_vec[3]);
        neon_FF(&a0,b0,c0,d0,x_vec0[4],s11,ac_vec[4]);
        neon_FF(&d0,a0,b0,c0,x_vec0[5],s12,ac_vec[5]);
        neon_FF(&c0,d0,a0,b0,x_vec0[6],s13,ac_vec[6]);
        neon_FF(&b0,c0,d0,a0,x_vec0[7],s14,ac_vec[7]);
        neon_FF(&a0,b0,c0,d0,x_vec0[8],s11,ac_vec[8]);
        neon_FF(&d0,a0,b0,c0,x_vec0[9],s12,ac_vec[9]);
        neon_FF(&c0,d0,a0,b0,x_vec0[10],s13,ac_vec[10]);
        neon_FF(&b0,c0,d0,a0,x_vec0[11],s14,ac_vec[11]);
        neon_FF(&a0,b0,c0,d0,x_vec0[12],s11,ac_vec[12]);
        neon_FF(&d0,a0,b0,c0,x_vec0[13],s12,ac_vec[13]);
        neon_FF(&c0,d0,a0,b0,x_vec0[14],s13,ac_vec[14]);
        neon_FF(&b0,c0,d0,a0,x_vec0[15],s14,ac_vec[15]);
        neon_FF(&a1,b1,c1,d1,x_vec1[0],s11,ac_vec[0]);
        neon_FF(&d1,a1,b1,c1,x_vec1[1],s12,ac_vec[1]);
        neon_FF(&c1,d1,a1,b1,x_vec1[2],s13,ac_vec[2]);
        neon_FF(&b1,c1,d1,a1,x_vec1[3],s14,ac_vec[3]);
        neon_FF(&a1,b1,c1,d1,x_vec1[4],s11,ac_vec[4]);
        neon_FF(&d1,a1,b1,c1,x_vec1[5],s12,ac_vec[5]);
        neon_FF(&c1,d1,a1,b1,x_vec1[6],s13,ac_vec[6]);
        neon_FF(&b1,c1,d1,a1,x_vec1[7],s14,ac_vec[7]);
        neon_FF(&a1,b1,c1,d1,x_vec1[8],s11,ac_vec[8]);
        neon_FF(&d1,a1,b1,c1,x_vec1[9],s12,ac_vec[9]);
        neon_FF(&c1,d1,a1,b1,x_vec1[10],s13,ac_vec[10]);
        neon_FF(&b1,c1,d1,a1,x_vec1[11],s14,ac_vec[11]);
        neon_FF(&a1,b1,c1,d1,x_vec1[12],s11,ac_vec[12]);
        neon_FF(&d1,a1,b1,c1,x_vec1[13],s12,ac_vec[13]);
        neon_FF(&c1,d1,a1,b1,x_vec1[14],s13,ac_vec[14]);
        neon_FF(&b1,c1,d1,a1,x_vec1[15],s14,ac_vec[15]);

        /* Round0 2 */
        neon_GG(&a0,b0,c0,d0,x_vec0[1],s21,ac_vec[16]);
        neon_GG(&d0,a0,b0,c0,x_vec0[6],s22,ac_vec[17]);
        neon_GG(&c0,d0,a0,b0,x_vec0[11],s23,ac_vec[18]);
        neon_GG(&b0,c0,d0,a0,x_vec0[0],s24,ac_vec[19]);
        neon_GG(&a0,b0,c0,d0,x_vec0[5],s21,ac_vec[20]);
        neon_GG(&d0,a0,b0,c0,x_vec0[10],s22,ac_vec[21]);
        neon_GG(&c0,d0,a0,b0,x_vec0[15],s23,ac_vec[22]);
        neon_GG(&b0,c0,d0,a0,x_vec0[4],s24,ac_vec[23]);
        neon_GG(&a0,b0,c0,d0,x_vec0[9],s21,ac_vec[24]);
        neon_GG(&d0,a0,b0,c0,x_vec0[14],s22,ac_vec[25]);
        neon_GG(&c0,d0,a0,b0,x_vec0[3],s23,ac_vec[26]);
        neon_GG(&b0,c0,d0,a0,x_vec0[8],s24,ac_vec[27]);
        neon_GG(&a0,b0,c0,d0,x_vec0[13],s21,ac_vec[28]);
        neon_GG(&d0,a0,b0,c0,x_vec0[2],s22,ac_vec[29]);
        neon_GG(&c0,d0,a0,b0,x_vec0[7],s23,ac_vec[30]);
        neon_GG(&b0,c0,d0,a0,x_vec0[12],s24,ac_vec[31]);
        neon_GG(&a1,b1,c1,d1,x_vec1[1],s21,ac_vec[16]);
        neon_GG(&d1,a1,b1,c1,x_vec1[6],s22,ac_vec[17]);
        neon_GG(&c1,d1,a1,b1,x_vec1[11],s23,ac_vec[18]);
        neon_GG(&b1,c1,d1,a1,x_vec1[0],s24,ac_vec[19]);
        neon_GG(&a1,b1,c1,d1,x_vec1[5],s21,ac_vec[20]);
        neon_GG(&d1,a1,b1,c1,x_vec1[10],s22,ac_vec[21]);
        neon_GG(&c1,d1,a1,b1,x_vec1[15],s23,ac_vec[22]);
        neon_GG(&b1,c1,d1,a1,x_vec1[4],s24,ac_vec[23]);
        neon_GG(&a1,b1,c1,d1,x_vec1[9],s21,ac_vec[24]);
        neon_GG(&d1,a1,b1,c1,x_vec1[14],s22,ac_vec[25]);
        neon_GG(&c1,d1,a1,b1,x_vec1[3],s23,ac_vec[26]);
        neon_GG(&b1,c1,d1,a1,x_vec1[8],s24,ac_vec[27]);
        neon_GG(&a1,b1,c1,d1,x_vec1[13],s21,ac_vec[28]);
        neon_GG(&d1,a1,b1,c1,x_vec1[2],s22,ac_vec[29]);
        neon_GG(&c1,d1,a1,b1,x_vec1[7],s23,ac_vec[30]);
        neon_GG(&b1,c1,d1,a1,x_vec1[12],s24,ac_vec[31]);


        /* Round0 3 */
        neon_HH(&a0,b0,c0,d0,x_vec0[5],s31,ac_vec[32]);
        neon_HH(&d0,a0,b0,c0,x_vec0[8],s32,ac_vec[33]);
        neon_HH(&c0,d0,a0,b0,x_vec0[11],s33,ac_vec[34]);
        neon_HH(&b0,c0,d0,a0,x_vec0[14],s34,ac_vec[35]);
        neon_HH(&a0,b0,c0,d0,x_vec0[1],s31,ac_vec[36]);
        neon_HH(&d0,a0,b0,c0,x_vec0[4],s32,ac_vec[37]);
        neon_HH(&c0,d0,a0,b0,x_vec0[7],s33,ac_vec[38]);
        neon_HH(&b0,c0,d0,a0,x_vec0[10],s34,ac_vec[39]);
        neon_HH(&a0,b0,c0,d0,x_vec0[13],s31,ac_vec[40]);
        neon_HH(&d0,a0,b0,c0,x_vec0[0],s32,ac_vec[41]);
        neon_HH(&c0,d0,a0,b0,x_vec0[3],s33,ac_vec[42]);
        neon_HH(&b0,c0,d0,a0,x_vec0[6],s34,ac_vec[43]);
        neon_HH(&a0,b0,c0,d0,x_vec0[9],s31,ac_vec[44]);
        neon_HH(&d0,a0,b0,c0,x_vec0[12],s32,ac_vec[45]);
        neon_HH(&c0,d0,a0,b0,x_vec0[15],s33,ac_vec[46]);
        neon_HH(&b0,c0,d0,a0,x_vec0[2],s34,ac_vec[47]);
        neon_HH(&a1,b1,c1,d1,x_vec1[5],s31,ac_vec[32]);
        neon_HH(&d1,a1,b1,c1,x_vec1[8],s32,ac_vec[33]);
        neon_HH(&c1,d1,a1,b1,x_vec1[11],s33,ac_vec[34]);
        neon_HH(&b1,c1,d1,a1,x_vec1[14],s34,ac_vec[35]);
        neon_HH(&a1,b1,c1,d1,x_vec1[1],s31,ac_vec[36]);
        neon_HH(&d1,a1,b1,c1,x_vec1[4],s32,ac_vec[37]);
        neon_HH(&c1,d1,a1,b1,x_vec1[7],s33,ac_vec[38]);
        neon_HH(&b1,c1,d1,a1,x_vec1[10],s34,ac_vec[39]);
        neon_HH(&a1,b1,c1,d1,x_vec1[13],s31,ac_vec[40]);
        neon_HH(&d1,a1,b1,c1,x_vec1[0],s32,ac_vec[41]);
        neon_HH(&c1,d1,a1,b1,x_vec1[3],s33,ac_vec[42]);
        neon_HH(&b1,c1,d1,a1,x_vec1[6],s34,ac_vec[43]);
        neon_HH(&a1,b1,c1,d1,x_vec1[9],s31,ac_vec[44]);
        neon_HH(&d1,a1,b1,c1,x_vec1[12],s32,ac_vec[45]);
        neon_HH(&c1,d1,a1,b1,x_vec1[15],s33,ac_vec[46]);
        neon_HH(&b1,c1,d1,a1,x_vec1[2],s34,ac_vec[47]);

        /* Round0 4 */
        neon_II(&a0,b0,c0,d0,x_vec0[0],s41,ac_vec[48]);
        neon_II(&d0,a0,b0,c0,x_vec0[7],s42,ac_vec[49]);
        neon_II(&c0,d0,a0,b0,x_vec0[14],s43,ac_vec[50]);
        neon_II(&b0,c0,d0,a0,x_vec0[5],s44,ac_vec[51]);
        neon_II(&a0,b0,c0,d0,x_vec0[12],s41,ac_vec[52]);
        neon_II(&d0,a0,b0,c0,x_vec0[3],s42,ac_vec[53]);
        neon_II(&c0,d0,a0,b0,x_vec0[10],s43,ac_vec[54]);
        neon_II(&b0,c0,d0,a0,x_vec0[1],s44,ac_vec[55]);
        neon_II(&a0,b0,c0,d0,x_vec0[8],s41,ac_vec[56]);
        neon_II(&d0,a0,b0,c0,x_vec0[15],s42,ac_vec[57]);
        neon_II(&c0,d0,a0,b0,x_vec0[6],s43,ac_vec[58]);
        neon_II(&b0,c0,d0,a0,x_vec0[13],s44,ac_vec[59]);
        neon_II(&a0,b0,c0,d0,x_vec0[4],s41,ac_vec[60]);
        neon_II(&d0,a0,b0,c0,x_vec0[11],s42,ac_vec[61]);
        neon_II(&c0,d0,a0,b0,x_vec0[2],s43,ac_vec[62]);
        neon_II(&b0,c0,d0,a0,x_vec0[9],s44,ac_vec[63]);
        neon_II(&a1,b1,c1,d1,x_vec1[0],s41,ac_vec[48]);
        neon_II(&d1,a1,b1,c1,x_vec1[7],s42,ac_vec[49]);
        neon_II(&c1,d1,a1,b1,x_vec1[14],s43,ac_vec[50]);
        neon_II(&b1,c1,d1,a1,x_vec1[5],s44,ac_vec[51]);
        neon_II(&a1,b1,c1,d1,x_vec1[12],s41,ac_vec[52]);
        neon_II(&d1,a1,b1,c1,x_vec1[3],s42,ac_vec[53]);
        neon_II(&c1,d1,a1,b1,x_vec1[10],s43,ac_vec[54]);
        neon_II(&b1,c1,d1,a1,x_vec1[1],s44,ac_vec[55]);
        neon_II(&a1,b1,c1,d1,x_vec1[8],s41,ac_vec[56]);
        neon_II(&d1,a1,b1,c1,x_vec1[15],s42,ac_vec[57]);
        neon_II(&c1,d1,a1,b1,x_vec1[6],s43,ac_vec[58]);
        neon_II(&b1,c1,d1,a1,x_vec1[13],s44,ac_vec[59]);
        neon_II(&a1,b1,c1,d1,x_vec1[4],s41,ac_vec[60]);
        neon_II(&d1,a1,b1,c1,x_vec1[11],s42,ac_vec[61]);
        neon_II(&c1,d1,a1,b1,x_vec1[2],s43,ac_vec[62]);
        neon_II(&b1,c1,d1,a1,x_vec1[9],s44,ac_vec[63]);
	
			a0=vaddq_u32(a0,aa0);b0=vaddq_u32(b0,bb0);c0=vaddq_u32(c0,cc0);d0=vaddq_u32(d0,dd0);
			a1=vaddq_u32(a1,aa1);b1=vaddq_u32(b1,bb1);c1=vaddq_u32(c1,cc1);d1=vaddq_u32(d1,dd1);
		}
	
	states[0][0]=byte_swap(vgetq_lane_u32(a0,0));
    states[1][0]=byte_swap(vgetq_lane_u32(a0,1));
    states[2][0]=byte_swap(vgetq_lane_u32(a0,2));
    states[3][0]=byte_swap(vgetq_lane_u32(a0,3));

    states[0][1]=byte_swap(vgetq_lane_u32(b0,0));
    states[1][1]=byte_swap(vgetq_lane_u32(b0,1));
    states[2][1]=byte_swap(vgetq_lane_u32(b0,2));
    states[3][1]=byte_swap(vgetq_lane_u32(b0,3));

    states[0][2]=byte_swap(vgetq_lane_u32(c0,0));
    states[1][2]=byte_swap(vgetq_lane_u32(c0,1));
    states[2][2]=byte_swap(vgetq_lane_u32(c0,2));
    states[3][2]=byte_swap(vgetq_lane_u32(c0,3));

    states[0][3]=byte_swap(vgetq_lane_u32(d0,0));
    states[1][3]=byte_swap(vgetq_lane_u32(d0,1));
    states[2][3]=byte_swap(vgetq_lane_u32(d0,2));
    states[3][3]=byte_swap(vgetq_lane_u32(d0,3));

	states[4][0]=byte_swap(vgetq_lane_u32(a1,0));
    states[5][0]=byte_swap(vgetq_lane_u32(a1,1));
    states[6][0]=byte_swap(vgetq_lane_u32(a1,2));
    states[7][0]=byte_swap(vgetq_lane_u32(a1,3));

    states[4][1]=byte_swap(vgetq_lane_u32(b1,0));
    states[5][1]=byte_swap(vgetq_lane_u32(b1,1));
    states[6][1]=byte_swap(vgetq_lane_u32(b1,2));
    states[7][1]=byte_swap(vgetq_lane_u32(b1,3));

    states[4][2]=byte_swap(vgetq_lane_u32(c1,0));
    states[5][2]=byte_swap(vgetq_lane_u32(c1,1));
    states[6][2]=byte_swap(vgetq_lane_u32(c1,2));
    states[7][2]=byte_swap(vgetq_lane_u32(c1,3));

    states[4][3]=byte_swap(vgetq_lane_u32(d1,0));
    states[5][3]=byte_swap(vgetq_lane_u32(d1,1));
    states[6][3]=byte_swap(vgetq_lane_u32(d1,2));
    states[7][3]=byte_swap(vgetq_lane_u32(d1,3));
	
		// 7. 释放内存
		for (int i=0;i<8;i++) {
			delete[] paddedMessages[i];
		}
	}
