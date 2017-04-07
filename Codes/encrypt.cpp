#include "encrypt.h"
#include <string.h>
#include <stdio.h>
#include "random.hpp"
#include <time.h>
#include <math.h>
#include "server_app.hpp"

#ifdef _WIN32
#include <assert.h>
#include "log.h"
#include <WinSock2.h>
#else
#include <netinet/in.h>
#include <sys/types.h>
#include <cassert>

#endif

//////////////////////////////////////////////////////////////////////////////////
//常量定义
namespace gt{


//////////////////////////////////////////////////////////////////////////////////
//加密密钥
const unsigned int g_dwPacketKey=0xC77BC66B;

//发送映射
const unsigned char g_cbEncryptMap[ENCRYPT_MAPKEY_LEN]=
{
	0x70,0x2F,0x40,0x5F,0x44,0x8E,0x6E,0x45,0x7E,0xAB,0x2C,0x1F,0xB4,0xAC,0x9D,0x91,
	0x0D,0x36,0x9B,0x0B,0xD4,0xC4,0x39,0x74,0xBF,0x23,0x16,0x14,0x06,0xEB,0x04,0x3E,
	0x12,0x5C,0x8B,0xBC,0x61,0x63,0xF6,0xA5,0xE1,0x65,0xD8,0xF5,0x5A,0x07,0xF0,0x13,
	0xF2,0x20,0x6B,0x4A,0x24,0x59,0x89,0x64,0xD7,0x42,0x6A,0x5E,0x3D,0x0A,0x77,0xE0,
	0x80,0x27,0xB8,0xC5,0x8C,0x0E,0xFA,0x8A,0xD5,0x29,0x56,0x57,0x6C,0x53,0x67,0x41,
	0xE8,0x00,0x1A,0xCE,0x86,0x83,0xB0,0x22,0x28,0x4D,0x3F,0x26,0x46,0x4F,0x6F,0x2B,
	0x72,0x3A,0xF1,0x8D,0x97,0x95,0x49,0x84,0xE5,0xE3,0x79,0x8F,0x51,0x10,0xA8,0x82,
	0xC6,0xDD,0xFF,0xFC,0xE4,0xCF,0xB3,0x09,0x5D,0xEA,0x9C,0x34,0xF9,0x17,0x9F,0xDA,
	0x87,0xF8,0x15,0x05,0x3C,0xD3,0xA4,0x85,0x2E,0xFB,0xEE,0x47,0x3B,0xEF,0x37,0x7F,
	0x93,0xAF,0x69,0x0C,0x71,0x31,0xDE,0x21,0x75,0xA0,0xAA,0xBA,0x7C,0x38,0x02,0xB7,
	0x81,0x01,0xFD,0xE7,0x1D,0xCC,0xCD,0xBD,0x1B,0x7A,0x2A,0xAD,0x66,0xBE,0x55,0x33,
	0x03,0xDB,0x88,0xB2,0x1E,0x4E,0xB9,0xE6,0xC2,0xF7,0xCB,0x7D,0xC9,0x62,0xC3,0xA6,
	0xDC,0xA7,0x50,0xB5,0x4B,0x94,0xC0,0x92,0x4C,0x11,0x5B,0x78,0xD9,0xB1,0xED,0x19,
	0xE9,0xA1,0x1C,0xB6,0x32,0x99,0xA3,0x76,0x9E,0x7B,0x6D,0x9A,0x30,0xD6,0xA9,0x25,
	0xC7,0xAE,0x96,0x35,0xD0,0xBB,0xD2,0xC8,0xA2,0x08,0xF3,0xD1,0x73,0xF4,0x48,0x2D,
	0x90,0xCA,0xE2,0x58,0xC1,0x18,0x52,0xFE,0xDF,0x68,0x98,0x54,0xEC,0x60,0x43,0x0F
};

//接收映射
const unsigned char g_cbCrevasseMap[ENCRYPT_MAPKEY_LEN]=
{
	0x51,0xA1,0x9E,0xB0,0x1E,0x83,0x1C,0x2D,0xE9,0x77,0x3D,0x13,0x93,0x10,0x45,0xFF,
	0x6D,0xC9,0x20,0x2F,0x1B,0x82,0x1A,0x7D,0xF5,0xCF,0x52,0xA8,0xD2,0xA4,0xB4,0x0B,
	0x31,0x97,0x57,0x19,0x34,0xDF,0x5B,0x41,0x58,0x49,0xAA,0x5F,0x0A,0xEF,0x88,0x01,
	0xDC,0x95,0xD4,0xAF,0x7B,0xE3,0x11,0x8E,0x9D,0x16,0x61,0x8C,0x84,0x3C,0x1F,0x5A,
	0x02,0x4F,0x39,0xFE,0x04,0x07,0x5C,0x8B,0xEE,0x66,0x33,0xC4,0xC8,0x59,0xB5,0x5D,
	0xC2,0x6C,0xF6,0x4D,0xFB,0xAE,0x4A,0x4B,0xF3,0x35,0x2C,0xCA,0x21,0x78,0x3B,0x03,
	0xFD,0x24,0xBD,0x25,0x37,0x29,0xAC,0x4E,0xF9,0x92,0x3A,0x32,0x4C,0xDA,0x06,0x5E,
	0x00,0x94,0x60,0xEC,0x17,0x98,0xD7,0x3E,0xCB,0x6A,0xA9,0xD9,0x9C,0xBB,0x08,0x8F,
	0x40,0xA0,0x6F,0x55,0x67,0x87,0x54,0x80,0xB2,0x36,0x47,0x22,0x44,0x63,0x05,0x6B,
	0xF0,0x0F,0xC7,0x90,0xC5,0x65,0xE2,0x64,0xFA,0xD5,0xDB,0x12,0x7A,0x0E,0xD8,0x7E,
	0x99,0xD1,0xE8,0xD6,0x86,0x27,0xBF,0xC1,0x6E,0xDE,0x9A,0x09,0x0D,0xAB,0xE1,0x91,
	0x56,0xCD,0xB3,0x76,0x0C,0xC3,0xD3,0x9F,0x42,0xB6,0x9B,0xE5,0x23,0xA7,0xAD,0x18,
	0xC6,0xF4,0xB8,0xBE,0x15,0x43,0x70,0xE0,0xE7,0xBC,0xF1,0xBA,0xA5,0xA6,0x53,0x75,
	0xE4,0xEB,0xE6,0x85,0x14,0x48,0xDD,0x38,0x2A,0xCC,0x7F,0xB1,0xC0,0x71,0x96,0xF8,
	0x3F,0x28,0xF2,0x69,0x74,0x68,0xB7,0xA3,0x50,0xD0,0x79,0x1D,0xFC,0xCE,0x8A,0x8D,
	0x2E,0x62,0x30,0xEA,0xED,0x2B,0x26,0xB9,0x81,0x7C,0x46,0x89,0x73,0xA2,0xF7,0x72
};
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////

//构造函数
CEncrypt::CEncrypt():m_dwSendPacketCount(0),m_dwRecvPacketCount(0)
	,m_cbSendRound(0),m_cbRecvRound(0)
	,m_cbSendRoundEx(0),m_cbRecvRoundEx(0)
{
	memcpy(m_cbEncryptMap,g_cbEncryptMap,sizeof(m_cbEncryptMap));
	for (int n=2;n<100;n++)
	{
		short m=sqrt((float)n);
		int i=0;
		for (i=2;i<=m;i++)
		{
			if (n%i==0)
			{
				break;
			}
		}
		if (i>m)
		{
			m_vtStep.push_back(n);
		}
	}

}

//析构函数
CEncrypt::~CEncrypt()
{
}
//生成密文
bool CEncrypt::XorEncrypt(const char* pszSourceData, char* pszEncrypData, unsigned short wMaxCount)
{
	//变量定义
	//CT2CW strSrcData(pszSourceData);
	char szEncrypData[MAX_ENCRYPT_LEN+1]="";

	//生成密钥
	unsigned short wRandKey[ENCRYPT_KEY_LEN];
	wRandKey[0]=strlen(pszSourceData);
	for (unsigned short i=1;i<sizeof(wRandKey);i++) wRandKey[i]=Rand()%0xFFFF;

	//步骤准备
	unsigned short wTempCode=0;
	unsigned short wTimes=((wRandKey[0]+ENCRYPT_KEY_LEN-1)/ENCRYPT_KEY_LEN)*ENCRYPT_KEY_LEN;

	//参数效验
	if ((wTimes*8+1)>wMaxCount) return false;

	//生成密文
	for (unsigned short i=0;i<wTimes;i++)
	{
		if (i<wRandKey[0]) wTempCode=pszSourceData[i]^wRandKey[i%ENCRYPT_KEY_LEN];
		else wTempCode=wRandKey[i%ENCRYPT_KEY_LEN]^(unsigned short)(Rand()%0xFFFF);
		sprintf(szEncrypData+i*8,"%04X%04X",wRandKey[i%ENCRYPT_KEY_LEN],wTempCode);
	}

	//字符转换
	//CW2CT strEncrypData(szEncrypData);
	strncpy(pszEncrypData,szEncrypData,wMaxCount);

	return true;
}

//解开密文
bool CEncrypt::XorCrevasse(const char* pszEncrypData, char* pszSourceData, unsigned short wMaxCount)
{
	//设置结果
	pszSourceData[0]=0;

	//变量定义
	//CT2CW strEncrypData(pszEncrypData);
	char szSrcData[MAX_SOURCE_LEN]="";

	//效验长度
	unsigned short wEncrypPassLen=strlen(pszEncrypData);
	if (wEncrypPassLen<ENCRYPT_KEY_LEN*8) return false;

	//提取长度
	char szTempBuffer[5]="";
	szTempBuffer[sizeof(szTempBuffer)-1]=0;
	memcpy(szTempBuffer,(const char*)pszEncrypData,sizeof(char)*4);

	//获取长度
	char * pEnd=NULL;
	unsigned short wSoureLength=(unsigned short)strtol(szTempBuffer,&pEnd,16);

	//长度效验
	//ASSERT(wEncrypPassLen==(((wSoureLength+ENCRYPT_KEY_LEN-1)/ENCRYPT_KEY_LEN)*ENCRYPT_KEY_LEN*8));
	if (wEncrypPassLen!=(((wSoureLength+ENCRYPT_KEY_LEN-1)/ENCRYPT_KEY_LEN)*ENCRYPT_KEY_LEN*8)) return false;

	//长度效验
	//ASSERT((wSoureLength+1)<=wMaxCount);
	if ((wSoureLength+1)>wMaxCount) return false;

	//解开密码
	for (int i=0;i<wSoureLength;i++)
	{
		//获取密钥
		char szKeyBuffer[5];
		szKeyBuffer[sizeof(szKeyBuffer)-1]=0;
		szTempBuffer[sizeof(szTempBuffer)-1]=0;
		memcpy(szKeyBuffer,(const char*)(pszEncrypData+i*8),sizeof(char)*4);
		memcpy(szTempBuffer,(const char*)pszEncrypData+i*8+4,sizeof(char)*4);

		//提取密钥
		char wKey=(char)strtol(szKeyBuffer,&pEnd,16);
		char wEncrypt=(char)strtol(szTempBuffer,&pEnd,16);

		//生成原文
		szSrcData[i]=(char)((char)wKey^(char)wEncrypt);
	}

	//终止字符
	szSrcData[wSoureLength]=0;

	//字符转换
	strncpy(pszSourceData,szSrcData,wMaxCount);

	return true;
}

//生成密文
bool CEncrypt::MapEncrypt(const char* pszSourceData, char* pszEncrypData, unsigned short wMaxCount)
{
	//效验参数
	if(wMaxCount<=strlen(pszEncrypData)) return false;
	if((pszEncrypData==NULL)||(pszSourceData==NULL)) return false;

	//变量定义
	int nLength=strlen(pszSourceData);
	unsigned char * pcbEncrypData=(unsigned char *)pszEncrypData;
	unsigned char * pcbSourceData=(unsigned char *)pszSourceData;

	//解密数据
	for (unsigned int i=0;i<nLength*sizeof(char);i++)
	{
		unsigned char cbIndex=pcbSourceData[i];
		pcbEncrypData[i]=g_cbEncryptMap[cbIndex];
	}

	//设置结果
	pszEncrypData[nLength]=0;

	return true;
}

//解开密文
bool CEncrypt::MapCrevasse(const char* pszEncrypData, char* pszSourceData, unsigned short wMaxCount)
{
	//效验参数
	if(wMaxCount<=strlen(pszEncrypData)) return false;
	if((pszEncrypData==NULL)||(pszSourceData==NULL)) return false;

	//变量定义
	int nLength=strlen(pszEncrypData);
	unsigned char * pcbEncrypData=(unsigned char *)pszEncrypData;
	unsigned char * pcbSourceData=(unsigned char *)pszSourceData;

	//解密数据
	for (unsigned int i=0;i<nLength*sizeof(char);i++)
	{
		unsigned char cbIndex=pcbEncrypData[i];
		pcbSourceData[i]=g_cbCrevasseMap[cbIndex];
	}

	//设置结果
	pszSourceData[nLength]=0;

	return true;
}
unsigned short CEncrypt::EncryptNetBuffer(unsigned char pcbDataBuffer[], unsigned short wDataSize, unsigned short wBufferSize,bool isClient)
{
	if (isClient)
	{
		if (m_dwSendPacketCount == 0)
		{
			return EncryptData(pcbDataBuffer,wDataSize,wBufferSize) ;
		}
		return EncryptDataEx(pcbDataBuffer,wDataSize,wBufferSize) ;
	}
	return EncryptDataEx(pcbDataBuffer,wDataSize,wBufferSize) ;
}
unsigned short CEncrypt::CrevasseNetBuffer(unsigned char pcbDataBuffer[], unsigned short wDataSize,bool isClient)
{
	if (isClient)
	{
		return CrevasseDataEx(pcbDataBuffer,wDataSize);
	}
	if (m_dwRecvPacketCount == 0)
	{
		return  CrevasseData(pcbDataBuffer,wDataSize);
	}
	return CrevasseDataEx(pcbDataBuffer,wDataSize);
}
//加密数据
unsigned short CEncrypt::EncryptData(unsigned char pcbDataBuffer[], unsigned short wDataSize, unsigned short wBufferSize)
{
	unsigned short i = 0;
	//效验参数
	if(wDataSize < (BINARY_PACKLEN_LEN + BINARY_PACKET_CMD_LEN)) return 0;
	if(wDataSize > BINARY_PACKAGE_MAXLEN) return 0;
	if(wBufferSize < (wDataSize + BINARY_PACKLEN_LEN  + BINARY_PACKET_CMD_LEN)) return 0;

	//调整长度
	unsigned short wEncryptSize = wDataSize+BINARY_PACKLEN_CHECK_LEN-BINARY_PACKLEN_LEN, wSnapCount = 0;
	if ((wEncryptSize % sizeof(unsigned int)) != 0)
	{
		wSnapCount = sizeof(unsigned int) - wEncryptSize % sizeof(unsigned int);
		memset(pcbDataBuffer + BINARY_PACKLEN_LEN  + wEncryptSize, 0, wSnapCount);
	}

	//效验码与字节映射
	unsigned char cbCheckCode = 0;
	for (i = BINARY_PACKLEN_LEN; i < wDataSize; i++)
	{
		cbCheckCode += pcbDataBuffer[i];
		pcbDataBuffer[i] = MapSendByte(pcbDataBuffer[i]);
	}
	//末尾插入checkcode
	*(pcbDataBuffer+wDataSize)= ~cbCheckCode + 1;
	
	//添加一个checkcode
	wDataSize+=BINARY_PACKLEN_CHECK_LEN;

	//创建密钥
	unsigned int dwXorKey = m_dwSendXorKey;
	if (m_dwSendPacketCount == 0)
	{
		//生成第一次随机种子
		dwXorKey = (unsigned int)time(0);
		//随机映射种子
		dwXorKey = SeedRandMap((unsigned short)dwXorKey);
		dwXorKey |= ((unsigned int)SeedRandMap((unsigned short)(dwXorKey >> 16))) << 16;
		dwXorKey ^= g_dwPacketKey;
		m_dwSendXorKey = dwXorKey;
	}

	//加密数据
	unsigned short * pwSeed = (unsigned short *)(pcbDataBuffer + BINARY_PACKLEN_LEN);
	unsigned int * pdwXor = (unsigned int *)(pcbDataBuffer + BINARY_PACKLEN_LEN);
	unsigned short wEncrypCount = (wEncryptSize + wSnapCount) / sizeof(unsigned int);
	//小端
#if (0x1234&0xFF)==0x12
	unsigned int tmpXor=ntohl(*pdwXor);
	unsigned short tmpSeed=0;
	for (i = 0; i < wEncrypCount; i++)
	{
		tmpXor^= dwXorKey;
		*pdwXor++=htonl(tmpXor);
		tmpSeed=ntohs(*pwSeed++);
		dwXorKey = SeedRandMap(tmpSeed);
		tmpSeed=ntohs(*pwSeed++);
		dwXorKey |= ((unsigned int)SeedRandMap(tmpSeed)) << 16;
		dwXorKey ^= g_dwPacketKey;
		tmpXor=ntohl(*pdwXor);
	}
#else
	for (i = 0; i < wEncrypCount; i++)
	{
		*pdwXor++ ^= dwXorKey;
		dwXorKey = SeedRandMap(*pwSeed++);
		dwXorKey |= ((unsigned int)SeedRandMap(*pwSeed++)) << 16;
		dwXorKey ^= g_dwPacketKey;
	}
#endif

	//插入密钥
	if (m_dwSendPacketCount == 0)
	{
		memmove(pcbDataBuffer + BINARY_PACKLEN_LEN + sizeof(unsigned int), pcbDataBuffer + BINARY_PACKLEN_LEN , wDataSize);
//小端
#if (0x1234&0xFF)==0x12
		*((unsigned int *)(pcbDataBuffer + BINARY_PACKLEN_LEN)) =ntohl(m_dwSendXorKey);
#else
		*((unsigned int *)(pcbDataBuffer + BINARY_PACKLEN_LEN)) =m_dwSendXorKey;
#endif
		wDataSize += sizeof(unsigned int);
	}

	unsigned int* pSize = (unsigned int*)pcbDataBuffer;
//网络套接字转换
#if (0x1234&0xFF)!=0x12
	*pSize = wDataSize;
#else
	*pSize = htonl(wDataSize);
#endif
	

	//设置变量
	m_dwSendPacketCount++;
	m_dwSendXorKey = dwXorKey;

	return wDataSize;
}

//解密数据
unsigned short CEncrypt::CrevasseData(unsigned char pcbDataBuffer[], unsigned short wDataSize)
{
	unsigned short i = 0;
	//效验参数
	if(wDataSize < BINARY_PACKLEN_LEN + BINARY_PACKET_CMD_LEN) return 0;
	//网络套接字转换
	//小端
#if (0x1234&0xFF)!=0x12
	unsigned int tmpSize =*(unsigned int *)pcbDataBuffer;
	if(tmpSize != wDataSize) return 0;
#else
	unsigned int tmpSize =ntohl(*(unsigned int *)pcbDataBuffer) ;
	if(tmpSize != wDataSize) return 0;
#endif
	///*******************************/
	//DEBUG_LOGEX(1,"CrevasseData/*******************************/%d before\n",wDataSize);
	//string str="";
	//char buffer[16]="";
	//for (int i = 4;i<wDataSize;++i)
	//{
	//	sprintf(buffer,"%02x ",pcbDataBuffer[i]);
	//	str+=buffer;
	//}
	//DEBUG_LOGEX(1,"[%s]\n",str.c_str());
	//调整长度
	unsigned short wSnapCount = 0;
	if ((wDataSize % sizeof(unsigned int)) != 0)
	{
		wSnapCount = sizeof(unsigned int) - wDataSize % sizeof(unsigned int);
		memset(pcbDataBuffer + wDataSize, 0, wSnapCount);
	}

	//提取密钥
	if (m_dwRecvPacketCount == 0)
	{
		if(wDataSize < (BINARY_PACKLEN_LEN + sizeof(unsigned int))) return 0;
#if (0x1234&0xFF)==0x12
		m_dwRecvXorKey = ntohl(*(unsigned int *)(pcbDataBuffer + BINARY_PACKLEN_LEN));
#else
		m_dwRecvXorKey = *(unsigned int *)(pcbDataBuffer + BINARY_PACKLEN_LEN);
#endif
		memmove(pcbDataBuffer + BINARY_PACKLEN_LEN, pcbDataBuffer + BINARY_PACKLEN_LEN + sizeof(unsigned int),
			wDataSize - BINARY_PACKLEN_LEN - sizeof(unsigned int));
		wDataSize -= sizeof(unsigned int);
	}

	//解密数据
	unsigned int dwXorKey = m_dwRecvXorKey;
	unsigned int * pdwXor = (unsigned int *)(pcbDataBuffer + BINARY_PACKLEN_LEN );
	unsigned short  * pwSeed = (unsigned short *)(pcbDataBuffer + BINARY_PACKLEN_LEN );
	unsigned short wEncrypCount = (wDataSize + wSnapCount - BINARY_PACKLEN_LEN ) / 4;

#if (0x1234&0xFF)==0x12
	unsigned int tmpXor=0;
	unsigned short tmpSeed=0;
	for (i = 0; i < wEncrypCount; i++)
	{
		if ((i == (wEncrypCount - 1)) && (wSnapCount > 0))
		{
			unsigned char * pcbKey = ((unsigned char *) & m_dwRecvXorKey) + sizeof(unsigned int) - wSnapCount;
			memcpy(pcbDataBuffer + wDataSize, pcbKey, wSnapCount);
		}
		tmpSeed=ntohs(*pwSeed++);
		dwXorKey = SeedRandMap(tmpSeed);
		tmpSeed=ntohs(*pwSeed++);
		dwXorKey |= ((unsigned int)SeedRandMap(tmpSeed)) << 16;
		dwXorKey ^= g_dwPacketKey;
		
		tmpXor=ntohl(*pdwXor);
		tmpXor ^= m_dwRecvXorKey;
		*pdwXor++=htonl(tmpXor);
		m_dwRecvXorKey = dwXorKey;
	}
#else
	for (i = 0; i < wEncrypCount; i++)
	{
		if ((i == (wEncrypCount - 1)) && (wSnapCount > 0))
		{
			unsigned char * pcbKey = ((unsigned char *) & m_dwRecvXorKey) + sizeof(unsigned int) - wSnapCount;
			memcpy(pcbDataBuffer + wDataSize, pcbKey, wSnapCount);
		}
		dwXorKey = SeedRandMap(*pwSeed++);
		dwXorKey |= ((unsigned int)SeedRandMap(*pwSeed++)) << 16;
		dwXorKey ^= g_dwPacketKey;
		*pdwXor++ ^= m_dwRecvXorKey;
		m_dwRecvXorKey = dwXorKey;
	}
#endif
	////*///*******************************/
	//str="";
	//DEBUG_LOGEX(1,"CrevasseData/*******************************/after %d\n",wDataSize);
	//for (int i = 4;i<wDataSize;++i)
	//{
	//	sprintf(buffer,"%02x ",pcbDataBuffer[i]);
	//	str+=buffer;
	//}
	//DEBUG_LOGEX(1,"[%s]\n",str.c_str());

	///*******************************/*/

	//减去一个checkcode
	wDataSize-=BINARY_PACKLEN_CHECK_LEN;

	//效验码与字节映射
	unsigned char cbCheckCode = *(pcbDataBuffer+wDataSize);

	for (i = BINARY_PACKLEN_LEN; i < wDataSize; i++)
	{
		pcbDataBuffer[i] = MapRecvByte(pcbDataBuffer[i]);
		cbCheckCode += pcbDataBuffer[i];
	}
	//str="";
	//DEBUG_LOGEX(1,"CrevasseData/*******************************/change %d\n",wDataSize);
	//for (int i = 4;i<wDataSize;++i)
	//{
	//	sprintf(buffer,"%02x ",pcbDataBuffer[i]);
	//	str+=buffer;
	//}
	//DEBUG_LOGEX(1,"[%s]\n",str.c_str());
	if (cbCheckCode != 0) 
	{
		ERROR_LOG("network data error,some data changed!");
		return 0;
	}
	unsigned int* pSize = (unsigned int*)pcbDataBuffer;
#if (0x1234&0xFF)!=0x12
	*pSize = wDataSize;
#else
	*pSize = htonl(wDataSize);
#endif

	m_dwRecvPacketCount++;
	return wDataSize;
}


//加密数据
unsigned short CEncrypt::EncryptDataEx(unsigned char pcbDataBuffer[], unsigned short wDataSize, unsigned short wBufferSize)
{
	unsigned short i = 0;
	//效验参数
	if(wDataSize < (BINARY_PACKLEN_LEN  + BINARY_PACKET_CMD_LEN)) return 0;
	if(wDataSize > BINARY_PACKAGE_MAXLEN) return 0;
	if(wBufferSize < (wDataSize + BINARY_PACKLEN_LEN  + BINARY_PACKET_CMD_LEN)) return 0;

	//效验码与字节映射
	unsigned char cbCheckCode = 0;
	for (i = BINARY_PACKLEN_LEN; i < wDataSize; i++)
	{
		cbCheckCode += pcbDataBuffer[i];
		pcbDataBuffer[i] = MapSendByteEx(pcbDataBuffer[i]);
	}
	//末尾插入checkcode
	*(pcbDataBuffer+wDataSize)= ~cbCheckCode + 1;
//	GT_DEBUG("Encrypt:c%d,p%d,r%d,s%d",*(pcbDataBuffer+wDataSize),m_dwSendPacketCount,m_cbSendRoundEx,wDataSize);
	//添加一个checkcode
	wDataSize+=BINARY_PACKLEN_CHECK_LEN;

	///*******************************/
	//创建密钥
	unsigned int dwXorKey = m_dwSendXorKey;
	if (m_dwSendPacketCount == 0)
	{
		//生成第一次随机种子
		dwXorKey = (unsigned int)time(0);
		//随机映射种子
		dwXorKey = SeedRandMap((unsigned short)dwXorKey);
		dwXorKey |= ((unsigned int)SeedRandMap((unsigned short)(dwXorKey >> 16))) << 16;
		dwXorKey ^= g_dwPacketKey;
		m_dwSendXorKey = dwXorKey;
	}
	//插入密钥
	unsigned short tmp_movelen=BINARY_PACKLEN_LEN;
	if (m_dwSendPacketCount == 0)
	{
		tmp_movelen+=sizeof(unsigned int);
		memmove(pcbDataBuffer + tmp_movelen  + sizeof(m_cbEncryptMap)+sizeof(m_cbCrevasseMap)+ENCRYPT_STEP_LEN, pcbDataBuffer + BINARY_PACKLEN_LEN , wDataSize);
		*(pcbDataBuffer + tmp_movelen) = m_cbRoundStep;
		memcpy(pcbDataBuffer + tmp_movelen +ENCRYPT_STEP_LEN,m_cbEncryptMap,sizeof(m_cbEncryptMap));
		memcpy(pcbDataBuffer + tmp_movelen +ENCRYPT_STEP_LEN+sizeof(m_cbEncryptMap),m_cbCrevasseMap,sizeof(m_cbCrevasseMap));
	
		//网络套接字转换
#if (0x1234&0xFF)==0x12
		*((unsigned int *)(pcbDataBuffer + BINARY_PACKLEN_LEN)) =ntohl(m_dwSendXorKey) ;
#else
		*((unsigned int *)(pcbDataBuffer + BINARY_PACKLEN_LEN)) = m_dwSendXorKey;
#endif
		
		wDataSize += sizeof(m_cbEncryptMap)+sizeof(m_cbCrevasseMap)+ENCRYPT_STEP_LEN+sizeof(unsigned int);
	}
	///////*******************************/
	//string str="";
	//char buffer[16]="";
	//DEBUG_LOGEX(2,"EncryptDataEx==============[code:%u;key:%u]======================before\n",*(pcbDataBuffer+wDataSize),m_dwSendXorKey);
	//for (int i = 4;i<wDataSize;++i)
	//{
	//	sprintf(buffer,"%02x ",pcbDataBuffer[i]);
	//	str+=buffer;
	//}
	//DEBUG_LOGEX(2,"[%s]\n",str.c_str());
	//调整长度
	unsigned short wEncryptSize = wDataSize - tmp_movelen, wSnapCount = 0;
	if ((wEncryptSize % sizeof(unsigned int)) != 0)
	{
		wSnapCount = sizeof(unsigned int) - wEncryptSize % sizeof(unsigned int);
		memset(pcbDataBuffer + tmp_movelen + wEncryptSize, 0, wSnapCount);
	}

	//加密数据
	unsigned short * pwSeed = (unsigned short *)(pcbDataBuffer + tmp_movelen );
	unsigned int * pdwXor = (unsigned int *)(pcbDataBuffer + tmp_movelen );
	unsigned short wEncrypCount = (wEncryptSize + wSnapCount) / sizeof(unsigned int);
	//小端
#if (0x1234&0xFF)==0x12
	unsigned int tmpXor=ntohl(*pdwXor);
	unsigned short tmpSeed=0;
	for (i = 0; i < wEncrypCount; i++)
	{
		tmpXor^= dwXorKey;
		*pdwXor++=htonl(tmpXor);
		tmpSeed=ntohs(*pwSeed++);
		dwXorKey = SeedRandMap(tmpSeed);
		tmpSeed=ntohs(*pwSeed++);
		dwXorKey |= ((unsigned int)SeedRandMap(tmpSeed)) << 16;
		dwXorKey ^= g_dwPacketKey;
		tmpXor=ntohl(*pdwXor);
	}
#else
	for (i = 0; i < wEncrypCount; i++)
	{
		*pdwXor++ ^= dwXorKey;
		dwXorKey = SeedRandMap(*pwSeed++);
		dwXorKey |= ((unsigned int)SeedRandMap(*pwSeed++)) << 16;
		dwXorKey ^= g_dwPacketKey;
	}
#endif
	/////*******************************/
	//str="";
	//DEBUG_LOGEX(2,"EncryptDataEx=========================================================================after\n");
	//for (int i = 4;i<wDataSize;++i)
	//{
	//	sprintf(buffer,"%02x ",pcbDataBuffer[i]);
	//	str+=buffer;
	//}
	//DEBUG_LOGEX(2,"[%s]\n",str.c_str());

	///*******************************/
	unsigned int* pSize = (unsigned int*)pcbDataBuffer;
#if (0x1234&0xFF)!=0x12
	*pSize = wDataSize;
#else
	*pSize = htonl(wDataSize);
#endif
	//设置变量
	m_dwSendPacketCount++;
	m_dwSendXorKey = dwXorKey;
	return wDataSize;
}

//解密数据
unsigned short CEncrypt::CrevasseDataEx(unsigned char pcbDataBuffer[], unsigned short wDataSize)
{
	unsigned short i = 0;
	//效验参数
	if(wDataSize < BINARY_PACKLEN_LEN + BINARY_PACKET_CMD_LEN) return 0;
	//网络套接字转换
	//小端
#if (0x1234&0xFF)!=0x12
	unsigned int tmpSize =*(unsigned int *)pcbDataBuffer;
	if(tmpSize != wDataSize) return 0;
#else
	unsigned int tmpSize =ntohl(*(unsigned int *)pcbDataBuffer) ;
	if(tmpSize != wDataSize) return 0;
#endif
	///////*******************************/
	//DEBUG_LOGEX(11,"CrevasseDataEx/*******************************/before %d\n",wDataSize);
	//string str="";
	//char buffer[16]="";
	//for (int i = 4;i<wDataSize;++i)
	//{
	//	sprintf(buffer,"%02x ",pcbDataBuffer[i]);
	//	str+=buffer;
	//}
	//DEBUG_LOGEX(11,"[%s]\n",str.c_str());
	/////*******************************/
	//调整长度
	unsigned short wSnapCount = 0;
	if ((wDataSize % sizeof(unsigned int)) != 0)
	{
		wSnapCount = sizeof(unsigned int) - wDataSize % sizeof(unsigned int);
		memset(pcbDataBuffer + wDataSize, 0, wSnapCount);
	}

	//提取密钥
	unsigned short tmp_movelen=BINARY_PACKLEN_LEN;
	if (m_dwRecvPacketCount == 0)
	{
		tmp_movelen+=sizeof(unsigned int);
		if(wDataSize < (tmp_movelen + sizeof(m_cbEncryptMap)+sizeof(m_cbCrevasseMap)+ENCRYPT_STEP_LEN)) return 0;
#if (0x1234&0xFF)==0x12
		m_dwRecvXorKey = ntohl(*(unsigned int *)(pcbDataBuffer + BINARY_PACKLEN_LEN));
#else
		m_dwRecvXorKey = *(unsigned int *)(pcbDataBuffer + BINARY_PACKLEN_LEN);
#endif
	}

	//解密数据
	unsigned int dwXorKey = m_dwRecvXorKey;
	unsigned int * pdwXor = (unsigned int *)(pcbDataBuffer + tmp_movelen);
	unsigned short  * pwSeed = (unsigned short *)(pcbDataBuffer + tmp_movelen);
	unsigned short wEncrypCount = (wDataSize + wSnapCount - tmp_movelen) / 4;
#if (0x1234&0xFF)==0x12
	unsigned int tmpXor=0;
	unsigned short tmpSeed=0;
	for (i = 0; i < wEncrypCount; i++)
	{
		if ((i == (wEncrypCount - 1)) && (wSnapCount > 0))
		{
			unsigned char * pcbKey = ((unsigned char *) & m_dwRecvXorKey) + sizeof(unsigned int) - wSnapCount;
			memcpy(pcbDataBuffer + wDataSize, pcbKey, wSnapCount);
		}
		tmpSeed=ntohs(*pwSeed++);
		dwXorKey = SeedRandMap(tmpSeed);
		tmpSeed=ntohs(*pwSeed++);
		dwXorKey |= ((unsigned int)SeedRandMap(tmpSeed)) << 16;
		dwXorKey ^= g_dwPacketKey;

		tmpXor=ntohl(*pdwXor);
		tmpXor ^= m_dwRecvXorKey;
		*pdwXor++=htonl(tmpXor);
		m_dwRecvXorKey = dwXorKey;
	}
#else
	for (i = 0; i < wEncrypCount; i++)
	{
		if ((i == (wEncrypCount - 1)) && (wSnapCount > 0))
		{
			unsigned char * pcbKey = ((unsigned char *) & m_dwRecvXorKey) + sizeof(unsigned int) - wSnapCount;
			memcpy(pcbDataBuffer + wDataSize, pcbKey, wSnapCount);
		}
		dwXorKey = SeedRandMap(*pwSeed++);
		dwXorKey |= ((unsigned int)SeedRandMap(*pwSeed++)) << 16;
		dwXorKey ^= g_dwPacketKey;
		*pdwXor++ ^= m_dwRecvXorKey;
		m_dwRecvXorKey = dwXorKey;
	}
#endif
	/////*///*******************************/
	//str="";
	//DEBUG_LOGEX(11,"CrevasseDataEx/*******************************/after %d\n",wDataSize);
	//for (int i = 4;i<wDataSize;++i)
	//{
	//	sprintf(buffer,"%02x ",pcbDataBuffer[i]);
	//	str+=buffer;
	//}
	//DEBUG_LOGEX(11,"[%s]\n",str.c_str());
	///////*******************************/*/
	//提取映射表
	if (m_dwRecvPacketCount == 0)
	{
		m_cbRoundStep = *(pcbDataBuffer + tmp_movelen);
		memcpy(m_cbEncryptMap,pcbDataBuffer + tmp_movelen+ENCRYPT_STEP_LEN,sizeof(m_cbEncryptMap));
		memcpy(m_cbCrevasseMap,pcbDataBuffer + tmp_movelen+ENCRYPT_STEP_LEN+sizeof(m_cbEncryptMap),sizeof(m_cbCrevasseMap));

		memmove(pcbDataBuffer + BINARY_PACKLEN_LEN, pcbDataBuffer + tmp_movelen +ENCRYPT_STEP_LEN+sizeof(m_cbEncryptMap)+sizeof(m_cbCrevasseMap) ,
			wDataSize - tmp_movelen - ENCRYPT_STEP_LEN-sizeof(m_cbEncryptMap)-sizeof(m_cbCrevasseMap));

		wDataSize -= sizeof(m_cbEncryptMap)+sizeof(m_cbCrevasseMap)+ENCRYPT_STEP_LEN+sizeof(unsigned int);
	}
	//减去一个checkcode
	wDataSize-=BINARY_PACKLEN_CHECK_LEN;

	//效验码与字节映射
	unsigned char cbCheckCode = *(pcbDataBuffer+wDataSize);
	//GT_DEBUG("Crevass:c%d,p%d,r%d,s%d",cbCheckCode,m_dwRecvPacketCount,m_cbRecvRoundEx,wDataSize);
	for (i = BINARY_PACKLEN_LEN; i < wDataSize; i++)
	{
		pcbDataBuffer[i] = MapRecvByteEx(pcbDataBuffer[i]);
		cbCheckCode += pcbDataBuffer[i];
	}

	if (cbCheckCode != 0) 
	{
		ERROR_LOG("network data error,some data changed!");
		return 0;
	}
	unsigned int* pSize = (unsigned int*)pcbDataBuffer;

#if (0x1234&0xFF)!=0x12
		*pSize = wDataSize;
#else
		*pSize = htonl(wDataSize);
#endif
	m_dwRecvPacketCount++;
	return wDataSize;
}
unsigned short CEncrypt::SeedRandMap(unsigned short wSeed)
{
	unsigned int dwHold = wSeed;
	return (unsigned short)((dwHold = dwHold * 241103L + 2933101L) >> 16);
}

//映射发送数据
unsigned char CEncrypt::MapSendByte(unsigned char const cbData)
{
	unsigned char cbMap = g_cbEncryptMap[(unsigned char)(cbData+m_cbSendRound)];
	m_cbSendRound += 3;
	return cbMap;
}

//映射接收数据
unsigned char CEncrypt::MapRecvByte(unsigned char const cbData)
{
	unsigned char cbMap = g_cbCrevasseMap[cbData] - m_cbRecvRound;
	m_cbRecvRound += 3;
	return cbMap;
}
//////////////////////////////////////////////////////////////////////////////////
//映射发送数据
unsigned char CEncrypt::MapSendByteEx(unsigned char const cbData)
{
	unsigned char cbMap = m_cbEncryptMap[(unsigned char)(cbData+m_cbSendRoundEx)];
	m_cbSendRoundEx += m_cbRoundStep;
	return cbMap;
}

//映射接收数据
unsigned char CEncrypt::MapRecvByteEx(unsigned char const cbData)
{
	unsigned char cbMap = m_cbCrevasseMap[cbData] - m_cbRecvRoundEx;
	m_cbRecvRoundEx += m_cbRoundStep;
	return cbMap;
}
void CEncrypt::InitRandom()
{	
	m_cbRoundStep=m_vtStep[Rand()%m_vtStep.size()];
	for (short i=0;i<ENCRYPT_MAPKEY_LEN;i++)
	{
		unsigned char tmp=rand()%(ENCRYPT_MAPKEY_LEN-i);
		unsigned char tmp2=m_cbEncryptMap[ENCRYPT_MAPKEY_LEN-1-i];
		m_cbEncryptMap[ENCRYPT_MAPKEY_LEN-1-i]=m_cbEncryptMap[tmp];
		m_cbEncryptMap[tmp]=tmp2;
		
		m_cbCrevasseMap[m_cbEncryptMap[ENCRYPT_MAPKEY_LEN-1-i]]=ENCRYPT_MAPKEY_LEN-1-i;
		m_cbCrevasseMap[m_cbEncryptMap[tmp]]=tmp;
	}
}
//清空数据
void CEncrypt::ClearData()
{
	m_cbSendRound=0;						//字节映射
	m_cbRecvRound=0;						//字节映射
	m_dwSendPacketCount=0;					//发送计数
	m_dwRecvPacketCount=0;					//接受计数
	m_cbSendRoundEx=0;						//字节映射
	m_cbRecvRoundEx=0;						//字节映射
}
};