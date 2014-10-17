/*

OIcqCrypt.h





OICQ����

hyj@oicq.com

1999/12/24



  ʵ�������㷨:

  Hash�㷨: MD5,��ʵ��

  �Գ��㷨: DES,δʵ��

  �ǶԳ��㷨: RSA,δʵ��



*/







#ifndef _INCLUDED_OICQCRYPT_H_

#define _INCLUDED_OICQCRYPT_H_



#define MD5_DIGEST_LENGTH	16





char Md5Test(); /*����MD5�����Ƿ���rfc1321����*/



/*

	����const char *inBuffer��int length

	���char *outBuffer

	����length��Ϊ0,outBuffer�ĳ���ΪMD5_DIGEST_LENGTH(16char)

*/

void Md5HashBuffer( char *outBuffer, const char *inBuffer, int length);





/*

	����const char *inBuffer��int length

	���char *outBuffer

	����length��Ϊ0,outBuffer�ĳ���ΪMD5_DIGEST_LENGTH(16char)

*/





//pOutBuffer��pInBuffer��Ϊ8char, pKeyΪ16char

void TeaEncryptECB(const char *pInBuf, const char *pKey, char *pOutBuf);

void TeaDecryptECB(const char *pInBuf, const char *pKey, char *pOutBuf);



/*pKeyΪ16char*/

/*

	����:pInBufΪ����ܵ����Ĳ���(Body),nInBufLenΪpInBuf����;

	���:pOutBufΪ���ĸ�ʽ,pOutBufLenΪpOutBuf�ĳ�����8char�ı���,����ӦԤ��nInBufLen+17;

*/

/*TEA�����㷨,CBCģʽ*/

/*���ĸ�ʽ:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(7char)*/

void oi_symmetry_encrypt(const char* pInBuf, int nInBufLen, const char* pKey, char* pOutBuf, int *pOutBufLen);



/*pKeyΪ16char*/

/*

	����:pInBufΪ���ĸ�ʽ,nInBufLenΪpInBuf�ĳ�����8char�ı���; *pOutBufLenΪ���ջ������ĳ���

		�ر�ע��*pOutBufLenӦԤ�ý��ջ������ĳ���!

	���:pOutBufΪ����(Body),pOutBufLenΪpOutBuf�ĳ���,����ӦԤ��nInBufLen-10;

	����ֵ:�����ʽ��ȷ����TRUE;

*/

/*TEA�����㷨,CBCģʽ*/

/*���ĸ�ʽ:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(7char)*/

char oi_symmetry_decrypt(const char* pInBuf, int nInBufLen, const char* pKey, char* pOutBuf, int *pOutBufLen);





/*pKeyΪ16char*/

/*

	����:pInBufΪ����ܵ����Ĳ���(Body),nInBufLenΪpInBuf����;

	���:pOutBufΪ���ĸ�ʽ,pOutBufLenΪpOutBuf�ĳ�����8char�ı���,����ӦԤ��nInBufLen+17;

*/

/*TEA�����㷨,CBCģʽ*/

/*���ĸ�ʽ:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(7char)*/

void oi_symmetry_encrypt2(const char* pInBuf, int nInBufLen, const char* pKey, char* pOutBuf, int *pOutBufLen);



/*pKeyΪ16char*/

/*

	����:pInBufΪ���ĸ�ʽ,nInBufLenΪpInBuf�ĳ�����8char�ı���; *pOutBufLenΪ���ջ������ĳ���

		�ر�ע��*pOutBufLenӦԤ�ý��ջ������ĳ���!

	���:pOutBufΪ����(Body),pOutBufLenΪpOutBuf�ĳ���,����ӦԤ��nInBufLen-10;

	����ֵ:�����ʽ��ȷ����TRUE;

*/

/*TEA�����㷨,CBCģʽ*/

/*���ĸ�ʽ:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(7char)*/

char oi_symmetry_decrypt2(const char* pInBuf, int nInBufLen, const char* pKey, char* pOutBuf, int *pOutBufLen);



#endif

