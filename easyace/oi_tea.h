/*

OIcqCrypt.h





OICQ加密

hyj@oicq.com

1999/12/24



  实现下列算法:

  Hash算法: MD5,已实现

  对称算法: DES,未实现

  非对称算法: RSA,未实现



*/







#ifndef _INCLUDED_OICQCRYPT_H_

#define _INCLUDED_OICQCRYPT_H_



#define MD5_DIGEST_LENGTH	16





char Md5Test(); /*测试MD5函数是否按照rfc1321工作*/



/*

	输入const char *inBuffer、int length

	输出char *outBuffer

	其中length可为0,outBuffer的长度为MD5_DIGEST_LENGTH(16char)

*/

void Md5HashBuffer( char *outBuffer, const char *inBuffer, int length);





/*

	输入const char *inBuffer、int length

	输出char *outBuffer

	其中length可为0,outBuffer的长度为MD5_DIGEST_LENGTH(16char)

*/





//pOutBuffer、pInBuffer均为8char, pKey为16char

void TeaEncryptECB(const char *pInBuf, const char *pKey, char *pOutBuf);

void TeaDecryptECB(const char *pInBuf, const char *pKey, char *pOutBuf);



/*pKey为16char*/

/*

	输入:pInBuf为需加密的明文部分(Body),nInBufLen为pInBuf长度;

	输出:pOutBuf为密文格式,pOutBufLen为pOutBuf的长度是8char的倍数,至少应预留nInBufLen+17;

*/

/*TEA加密算法,CBC模式*/

/*密文格式:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(7char)*/

void oi_symmetry_encrypt(const char* pInBuf, int nInBufLen, const char* pKey, char* pOutBuf, int *pOutBufLen);



/*pKey为16char*/

/*

	输入:pInBuf为密文格式,nInBufLen为pInBuf的长度是8char的倍数; *pOutBufLen为接收缓冲区的长度

		特别注意*pOutBufLen应预置接收缓冲区的长度!

	输出:pOutBuf为明文(Body),pOutBufLen为pOutBuf的长度,至少应预留nInBufLen-10;

	返回值:如果格式正确返回TRUE;

*/

/*TEA解密算法,CBC模式*/

/*密文格式:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(7char)*/

char oi_symmetry_decrypt(const char* pInBuf, int nInBufLen, const char* pKey, char* pOutBuf, int *pOutBufLen);





/*pKey为16char*/

/*

	输入:pInBuf为需加密的明文部分(Body),nInBufLen为pInBuf长度;

	输出:pOutBuf为密文格式,pOutBufLen为pOutBuf的长度是8char的倍数,至少应预留nInBufLen+17;

*/

/*TEA加密算法,CBC模式*/

/*密文格式:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(7char)*/

void oi_symmetry_encrypt2(const char* pInBuf, int nInBufLen, const char* pKey, char* pOutBuf, int *pOutBufLen);



/*pKey为16char*/

/*

	输入:pInBuf为密文格式,nInBufLen为pInBuf的长度是8char的倍数; *pOutBufLen为接收缓冲区的长度

		特别注意*pOutBufLen应预置接收缓冲区的长度!

	输出:pOutBuf为明文(Body),pOutBufLen为pOutBuf的长度,至少应预留nInBufLen-10;

	返回值:如果格式正确返回TRUE;

*/

/*TEA解密算法,CBC模式*/

/*密文格式:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(7char)*/

char oi_symmetry_decrypt2(const char* pInBuf, int nInBufLen, const char* pKey, char* pOutBuf, int *pOutBufLen);



#endif

