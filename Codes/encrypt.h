#ifndef __ENCRYPT_HEAD_FILE__
#define __ENCRYPT_HEAD_FILE__
//////////////////////////////////////////////////////////////////////////////////
#include <vector>
using namespace std;
//////////////////////////////////////////////////////////////////////////////////
namespace gt{
//宏定义
//xor
#define XOR_TIMES					8									//加密倍数
#define MAX_SOURCE_LEN				64									//最大长度
#define MAX_ENCRYPT_LEN				(MAX_SOURCE_LEN*XOR_TIMES)			//最大长度
#define ENCRYPT_KEY_LEN				8									//密钥长度

//encrypt
//密钥长度
#define ENCRYPT_MAPKEY_LEN				256								//密钥映射表长度
#define ENCRYPT_STEP_LEN				sizeof(unsigned char)			//密钥步长度

	enum {
		TEXT_PACKLEN_LEN		= 4,
		TEXT_PACKAGE_MAXLEN		= 0xffff,
		BINARY_PACKLEN_LEN		= 4,
		BINARY_PACKET_CMD_LEN	= 2,
		BINARY_PACKLEN_CHECK_LEN =1,
		BINARY_PACKAGE_MAXLEN	= 0xffff,
		BINARY_PACKLEN_LEN2     = 2,
		TEXT_PACKLEN_LEN_2		= 6,
		TEXT_PACKAGE_MAXLEN_2	= 0xffffff,
	};

//加密组件
class CEncrypt
{
	//函数定义
public:
	//构造函数
	CEncrypt();
	//析构函数
	virtual ~CEncrypt();

	//加密函数
public:
	//生成密文
	static bool XorEncrypt(const char* pszSourceData, char* pszEncrypData, unsigned short wMaxCount);
	//解开密文
	static bool XorCrevasse(const char* pszEncrypData, char* pszSourceData, unsigned short wMaxCount);

	//加密函数
public:
	//生成密文
	static bool MapEncrypt(const char* pszSourceData, char* pszEncrypData, unsigned short wMaxCount);
	//解开密文
	static bool MapCrevasse(const char* pszEncrypData, char* pszSourceData, unsigned short wMaxCount);

public:
	unsigned short EncryptNetBuffer(unsigned char pcbDataBuffer[], unsigned short wDataSize, unsigned short wBufferSize,bool isClient=false);
	unsigned short CrevasseNetBuffer(unsigned char pcbDataBuffer[], unsigned short wDataSize,bool isClient=false);

private:
	unsigned short EncryptData(unsigned char pcbDataBuffer[], unsigned short wDataSize, unsigned short wBufferSize);
	unsigned short CrevasseData(unsigned char pcbDataBuffer[], unsigned short wDataSize);
	unsigned short EncryptDataEx(unsigned char pcbDataBuffer[], unsigned short wDataSize, unsigned short wBufferSize);
	unsigned short CrevasseDataEx(unsigned char pcbDataBuffer[], unsigned short wDataSize);
	//随机映射
	inline unsigned short SeedRandMap(unsigned short wSeed);
	//映射发送数据
	inline unsigned char MapSendByte(unsigned char const cbData);
	//映射接收数据
	inline unsigned char MapRecvByte(unsigned char const cbData);
	//映射发送数据
	 unsigned char MapSendByteEx(unsigned char const cbData);
	//映射接收数据
	 unsigned char MapRecvByteEx(unsigned char const cbData);



public:
	//初始化
	void InitRandom();
	//清空数据
	void ClearData();

protected:
	unsigned char							m_cbSendRound;						//字节映射
	unsigned char							m_cbRecvRound;						//字节映射
	unsigned int							m_dwSendXorKey;						//发送密钥
	unsigned int							m_dwRecvXorKey;						//接收密钥
	unsigned int							m_dwSendPacketCount;				//发送计数
	unsigned int							m_dwRecvPacketCount;				//接受计数

private:
	unsigned char							m_cbEncryptMap[ENCRYPT_MAPKEY_LEN];
	unsigned char							m_cbCrevasseMap[ENCRYPT_MAPKEY_LEN];

	unsigned char							m_cbRoundStep;
	unsigned char							m_cbSendRoundEx;						//字节映射
	unsigned char							m_cbRecvRoundEx;						//字节映射
	vector<int>								m_vtStep;
};

//////////////////////////////////////////////////////////////////////////////////
};
#endif