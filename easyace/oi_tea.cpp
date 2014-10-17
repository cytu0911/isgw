#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>

#include "oi_tea.h"

#ifndef WIN32
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include "winsock2.h"
#endif


/*

�汾������ʷ



	JediHuang 2000��8��25�գ�

		����oi_symmetry_decrypt2��oi_symmetry_decrypt����;

		�����е�

			if ()

			{

				

			}

			

			if ()

			{

				

			}

		�߼�������

			if ()

			{

				

			}

			else if ()

			{

				

			}

		�߼�;







*/



/*

	void TeaEncryptECB(char *pInBuf, char *pKey, char *pOutBuf);

	void TeaDecryptECB(char *pInBuf, char *pKey, char *pOutBuf);

*/



typedef unsigned int WORD32;



const WORD32 DELTA = 0x9e3779b9;



#define ROUNDS 16

#define LOG_ROUNDS 4





/*pOutBuffer��pInBuffer��Ϊ8char, pKeyΪ16char*/

void TeaEncryptECB(const char *pInBuf, const char *pKey, char *pOutBuf)

{

	WORD32 y, z;

	WORD32 sum;

	WORD32 k[4];

	int i;



	/*plain-text is TCP/IP-endian;*/



	/*GetBlockBigEndian(in, y, z);*/

	y = ntohl(*((WORD32*)pInBuf));

	z = ntohl(*((WORD32*)(pInBuf+4)));

	/*TCP/IP network char order (which is big-endian).*/



	for ( i = 0; i<4; i++)

	{

		/*now key is TCP/IP-endian;*/

		k[i] = ntohl(*((WORD32*)(pKey+i*4)));

	}



	sum = 0;

	for (i=0; i<ROUNDS; i++)

	{   

		sum += DELTA;

		y += (z << 4) + k[0] ^ z + sum ^ (z >> 5) + k[1];

		z += (y << 4) + k[2] ^ y + sum ^ (y >> 5) + k[3];

	}







	*((WORD32*)pOutBuf) = htonl(y);

	*((WORD32*)(pOutBuf+4)) = htonl(z);

	



	/*now encrypted buf is TCP/IP-endian;*/

}



/*pOutBuffer��pInBuffer��Ϊ8char, pKeyΪ16char*/

void TeaDecryptECB(const char *pInBuf, const char *pKey, char *pOutBuf)

{

	WORD32 y, z, sum;

	WORD32 k[4];

	int i;



	/*now encrypted buf is TCP/IP-endian;*/

	/*TCP/IP network char order (which is big-endian).*/

	y = ntohl(*((WORD32*)pInBuf));

	z = ntohl(*((WORD32*)(pInBuf+4)));



	for ( i=0; i<4; i++)

	{

		/*key is TCP/IP-endian;*/

		k[i] = ntohl(*((WORD32*)(pKey+i*4)));

	}



	sum = DELTA << LOG_ROUNDS;

	for (i=0; i<ROUNDS; i++)

	{

		z -= (y << 4) + k[2] ^ y + sum ^ (y >> 5) + k[3]; 

		y -= (z << 4) + k[0] ^ z + sum ^ (z >> 5) + k[1];

		sum -= DELTA;

	}



	*((WORD32*)pOutBuf) = htonl(y);

	*((WORD32*)(pOutBuf+4)) = htonl(z);



	/*now plain-text is TCP/IP-endian;*/

}







#define SALT_LEN 2

#define ZERO_LEN 7



/*pKeyΪ16char*/

/*

	����:pInBufΪ����ܵ����Ĳ���(Body),nInBufLenΪpInBuf����;

	���:pOutBufΪ���ĸ�ʽ,pOutBufLenΪpOutBuf�ĳ�����8char�ı���;

*/

/*TEA�����㷨,CBCģʽ*/

/*���ĸ�ʽ:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(8char)*/

void oi_symmetry_encrypt(const char* pInBuf, int nInBufLen, const char* pKey, char* pOutBuf, int *pOutBufLen)

{

	

	int nPadSaltBodyZeroLen/*PadLen(1char)+Salt+Body+Zero�ĳ���*/;

	int nPadlen;

	char src_buf[8], zero_iv[8], *iv_buf;

	int src_i, i, j;



	/*����Body���ȼ���PadLen,��С���賤�ȱ���Ϊ8char��������*/

	nPadSaltBodyZeroLen = nInBufLen/*Body����*/+1+SALT_LEN+ZERO_LEN/*PadLen(1char)+Salt(2char)+Zero(8char)*/;

	if(nPadlen=nPadSaltBodyZeroLen%8) /*len=nSaltBodyZeroLen%8*/

	{

		/*ģ8��0�貹0,��1��7,��2��6,...,��7��1*/

		nPadlen=8-nPadlen;

	}



	/*srand( (unsigned)time( NULL ) ); ��ʼ�������*/

	/*���ܵ�һ������(8char),ȡǰ��10char*/

	src_buf[0] = ((char)rand()) & 0x0f8/*�����λ��PadLen,����*/ | (char)nPadlen;

	src_i = 1; /*src_iָ��src_buf��һ��λ��*/



	while(nPadlen--)

		src_buf[src_i++]=(char)rand(); /*Padding*/



	/*come here, i must <= 8*/



	memset(zero_iv, 0, 8);

	iv_buf = zero_iv; /*make iv*/



	*pOutBufLen = 0; /*init OutBufLen*/



	for (i=1;i<=SALT_LEN;) /*Salt(2char)*/

	{

		if (src_i<8)

		{

			src_buf[src_i++]=(char)rand();

			i++; /*i inc in here*/

		}



		if (src_i==8)

		{

			/*src_i==8*/

			

			for (j=0;j<8;j++) /*CBC XOR*/

				src_buf[j]^=iv_buf[j];

			/*pOutBuffer��pInBuffer��Ϊ8char, pKeyΪ16char*/

			TeaEncryptECB(src_buf, pKey, pOutBuf);

			src_i=0;

			iv_buf=pOutBuf;

			*pOutBufLen+=8;

			pOutBuf+=8;

		}

	}



	/*src_iָ��src_buf��һ��λ��*/



	while(nInBufLen)

	{

		if (src_i<8)

		{

			src_buf[src_i++]=*(pInBuf++);

			nInBufLen--;

		}



		if (src_i==8)

		{

			/*src_i==8*/

			

			for (i=0;i<8;i++) /*CBC XOR*/

				src_buf[i]^=iv_buf[i];

			/*pOutBuffer��pInBuffer��Ϊ8char, pKeyΪ16char*/

			TeaEncryptECB(src_buf, pKey, pOutBuf);

			src_i=0;

			iv_buf=pOutBuf;

			*pOutBufLen+=8;

			pOutBuf+=8;

		}

	}



	/*src_iָ��src_buf��һ��λ��*/



	for (i=1;i<=ZERO_LEN;)

	{

		if (src_i<8)

		{

			src_buf[src_i++]=0;

			i++; /*i inc in here*/

		}



		if (src_i==8)

		{

			/*src_i==8*/

			

			for (j=0;j<8;j++) /*CBC XOR*/

				src_buf[j]^=iv_buf[j];

			/*pOutBuffer��pInBuffer��Ϊ8char, pKeyΪ16char*/

			TeaEncryptECB(src_buf, pKey, pOutBuf);

			src_i=0;

			iv_buf=pOutBuf;

			*pOutBufLen+=8;

			pOutBuf+=8;

		}

	}



}



/*pKeyΪ16char*/

/*

	����:pInBufΪ���ĸ�ʽ,nInBufLenΪpInBuf�ĳ�����8char�ı���;

	���:pOutBufΪ����(Body),pOutBufLenΪpOutBuf�ĳ���;

	����ֵ:�����ʽ��ȷ����1;

*/

/*TEA�����㷨,CBCģʽ*/

/*���ĸ�ʽ:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(8char)*/

char oi_symmetry_decrypt(const char* pInBuf, int nInBufLen, const char* pKey, char* pOutBuf, int *pOutBufLen)

{



	int nPadLen, nPlainLen;

	char dest_buf[8];

	const char *iv_buf;

	int dest_i, i, j;



	//if (nInBufLen%8) return 0; BUG!

	if ((nInBufLen%8) || (nInBufLen<16)) return 0;

	



	TeaDecryptECB(pInBuf, pKey, dest_buf);



	nPadLen = dest_buf[0] & 0x7/*ֻҪ�����λ*/;



	/*���ĸ�ʽ:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(8char)*/

	i = nInBufLen-1/*PadLen(1char)*/-nPadLen-SALT_LEN-ZERO_LEN; /*���ĳ���*/

	if (*pOutBufLen<i) return 0;

	*pOutBufLen = i;

	if (*pOutBufLen < 0) return 0;

	



	iv_buf = pInBuf; /*init iv*/

	nInBufLen -= 8;

	pInBuf += 8;



	dest_i=1; /*dest_iָ��dest_buf��һ��λ��*/





	/*��Padding�˵�*/

	dest_i+=nPadLen;



	/*dest_i must <=8*/



	/*��Salt�˵�*/

	for (i=1; i<=SALT_LEN;)

	{

		if (dest_i<8)

		{

			dest_i++;

			i++;

		}

		else if (dest_i==8)

		{

			/*dest_i==8*/

			TeaDecryptECB(pInBuf, pKey, dest_buf);

			for (j=0; j<8; j++)

				dest_buf[j]^=iv_buf[j];

		

			iv_buf = pInBuf;

			nInBufLen -= 8;

			pInBuf += 8;

	

			dest_i=0; /*dest_iָ��dest_buf��һ��λ��*/

		}

	}



	/*��ԭ����*/



	nPlainLen=*pOutBufLen;

	while (nPlainLen)

	{

		if (dest_i<8)

		{

			*(pOutBuf++)=dest_buf[dest_i++];

			nPlainLen--;

		}

		else if (dest_i==8)

		{

			/*dest_i==8*/

			TeaDecryptECB(pInBuf, pKey, dest_buf);

			for (i=0; i<8; i++)

				dest_buf[i]^=iv_buf[i];

		

			iv_buf = pInBuf;

			nInBufLen -= 8;

			pInBuf += 8;

	

			dest_i=0; /*dest_iָ��dest_buf��һ��λ��*/

		}

	}



	/*У��Zero*/

	for (i=1;i<=ZERO_LEN;)

	{

		if (dest_i<8)

		{

			if(dest_buf[dest_i++]) return 0;

			i++;

		}

		else if (dest_i==8)

		{

			/*dest_i==8*/

			TeaDecryptECB(pInBuf, pKey, dest_buf);

			for (j=0; j<8; j++)

				dest_buf[j]^=iv_buf[j];

		

			iv_buf = pInBuf;

			nInBufLen -= 8;

			pInBuf += 8;

	

			dest_i=0; /*dest_iָ��dest_buf��һ��λ��*/

		}

	

	}



	return 1;

}



///////////////////////////////////////////////////////////////////////////////////////////////



/*pKeyΪ16char*/

/*

	����:pInBufΪ����ܵ����Ĳ���(Body),nInBufLenΪpInBuf����;

	���:pOutBufΪ���ĸ�ʽ,pOutBufLenΪpOutBuf�ĳ�����8char�ı���;

*/

/*TEA�����㷨,CBCģʽ*/

/*���ĸ�ʽ:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(8char)*/

void oi_symmetry_encrypt2(const char* pInBuf, int nInBufLen, const char* pKey, char* pOutBuf, int *pOutBufLen)

{

	

	int nPadSaltBodyZeroLen/*PadLen(1char)+Salt+Body+Zero�ĳ���*/;

	int nPadlen;

	char src_buf[8], iv_plain[8], *iv_crypt;

	int src_i, i, j;



	/*����Body���ȼ���PadLen,��С���賤�ȱ���Ϊ8char��������*/

	nPadSaltBodyZeroLen = nInBufLen/*Body����*/+1+SALT_LEN+ZERO_LEN/*PadLen(1char)+Salt(2char)+Zero(8char)*/;

	if(nPadlen=nPadSaltBodyZeroLen%8) /*len=nSaltBodyZeroLen%8*/

	{

		/*ģ8��0�貹0,��1��7,��2��6,...,��7��1*/

		nPadlen=8-nPadlen;

	}



	/*srand( (unsigned)time( NULL ) ); ��ʼ�������*/

	/*���ܵ�һ������(8char),ȡǰ��10char*/

	src_buf[0] = ((char)rand()) & 0x0f8/*�����λ��PadLen,����*/ | (char)nPadlen;

	src_i = 1; /*src_iָ��src_buf��һ��λ��*/



	while(nPadlen--)

		src_buf[src_i++]=(char)rand(); /*Padding*/



	/*come here, src_i must <= 8*/



	for ( i=0; i<8; i++)

		iv_plain[i] = 0;

	iv_crypt = iv_plain; /*make zero iv*/



	*pOutBufLen = 0; /*init OutBufLen*/



	for (i=1;i<=SALT_LEN;) /*Salt(2char)*/

	{

		if (src_i<8)

		{

			src_buf[src_i++]=(char)rand();

			i++; /*i inc in here*/

		}



		if (src_i==8)

		{

			/*src_i==8*/



			for (j=0;j<8;j++) /*����ǰ���ǰ8��char������(iv_cryptָ���)*/

				src_buf[j]^=iv_crypt[j];



			/*pOutBuffer��pInBuffer��Ϊ8char, pKeyΪ16char*/

			/*����*/

			TeaEncryptECB(src_buf, pKey, pOutBuf);



			for (j=0;j<8;j++) /*���ܺ����ǰ8��char������(iv_plainָ���)*/

				pOutBuf[j]^=iv_plain[j];



			/*���浱ǰ��iv_plain*/

			for (j=0;j<8;j++)

				iv_plain[j]=src_buf[j];



			/*����iv_crypt*/

			src_i=0;

			iv_crypt=pOutBuf;

			*pOutBufLen+=8;

			pOutBuf+=8;

		}

	}



	/*src_iָ��src_buf��һ��λ��*/



	while(nInBufLen)

	{

		if (src_i<8)

		{

			src_buf[src_i++]=*(pInBuf++);

			nInBufLen--;

		}



		if (src_i==8)

		{

			/*src_i==8*/

			

			for (j=0;j<8;j++) /*����ǰ���ǰ8��char������(iv_cryptָ���)*/

				src_buf[j]^=iv_crypt[j];

			/*pOutBuffer��pInBuffer��Ϊ8char, pKeyΪ16char*/

			TeaEncryptECB(src_buf, pKey, pOutBuf);



			for (j=0;j<8;j++) /*���ܺ����ǰ8��char������(iv_plainָ���)*/

				pOutBuf[j]^=iv_plain[j];



			/*���浱ǰ��iv_plain*/

			for (j=0;j<8;j++)

				iv_plain[j]=src_buf[j];



			src_i=0;

			iv_crypt=pOutBuf;

			*pOutBufLen+=8;

			pOutBuf+=8;

		}

	}



	/*src_iָ��src_buf��һ��λ��*/



	for (i=1;i<=ZERO_LEN;)

	{

		if (src_i<8)

		{

			src_buf[src_i++]=0;

			i++; /*i inc in here*/

		}



		if (src_i==8)

		{

			/*src_i==8*/

			

			for (j=0;j<8;j++) /*����ǰ���ǰ8��char������(iv_cryptָ���)*/

				src_buf[j]^=iv_crypt[j];

			/*pOutBuffer��pInBuffer��Ϊ8char, pKeyΪ16char*/

			TeaEncryptECB(src_buf, pKey, pOutBuf);



			for (j=0;j<8;j++) /*���ܺ����ǰ8��char������(iv_plainָ���)*/

				pOutBuf[j]^=iv_plain[j];



			/*���浱ǰ��iv_plain*/

			for (j=0;j<8;j++)

				iv_plain[j]=src_buf[j];



			src_i=0;

			iv_crypt=pOutBuf;

			*pOutBufLen+=8;

			pOutBuf+=8;

		}

	}



}



/*pKeyΪ16char*/

/*

	����:pInBufΪ���ĸ�ʽ,nInBufLenΪpInBuf�ĳ�����8char�ı���;

	���:pOutBufΪ����(Body),pOutBufLenΪpOutBuf�ĳ���;

	����ֵ:�����ʽ��ȷ����1;

*/

/*TEA�����㷨,CBCģʽ*/

/*���ĸ�ʽ:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(8char)*/

char oi_symmetry_decrypt2(const char* pInBuf, int nInBufLen, const char* pKey, char* pOutBuf, int *pOutBufLen)

{



	int nPadLen, nPlainLen;

	char dest_buf[8], zero_buf[8];

	const char *iv_pre_crypt, *iv_cur_crypt;

	int dest_i, i, j;



	//if (nInBufLen%8) return 0; BUG!

	if ((nInBufLen%8) || (nInBufLen<16)) return 0;

	



	TeaDecryptECB(pInBuf, pKey, dest_buf);



	nPadLen = dest_buf[0] & 0x7/*ֻҪ�����λ*/;



	/*���ĸ�ʽ:PadLen(1char)+Padding(var,0-7char)+Salt(2char)+Body(var char)+Zero(8char)*/

	i = nInBufLen-1/*PadLen(1char)*/-nPadLen-SALT_LEN-ZERO_LEN; /*���ĳ���*/

	if (*pOutBufLen<i) return 0;

	*pOutBufLen = i;

	if (*pOutBufLen < 0) return 0;

	

	for ( i=0; i<8; i++)

		zero_buf[i] = 0;



	iv_pre_crypt = zero_buf;

	iv_cur_crypt = pInBuf; /*init iv*/

	nInBufLen -= 8;

	pInBuf += 8;



	dest_i=1; /*dest_iָ��dest_buf��һ��λ��*/





	/*��Padding�˵�*/

	dest_i+=nPadLen;



	/*dest_i must <=8*/



	/*��Salt�˵�*/

	for (i=1; i<=SALT_LEN;)

	{

		if (dest_i<8)

		{

			dest_i++;

			i++;

		}

		else if (dest_i==8)

		{

			/*�⿪һ���µļ��ܿ�*/



			/*�ı�ǰһ�����ܿ��ָ��*/

			iv_pre_crypt = iv_cur_crypt;

			iv_cur_crypt = pInBuf; 



			/*���ǰһ������(��dest_buf[]��)*/

			for (j=0; j<8; j++)

				dest_buf[j]^=pInBuf[j];



			/*dest_i==8*/

			TeaDecryptECB(dest_buf, pKey, dest_buf);



			/*��ȡ����ʱ������ǰһ������(iv_pre_crypt)*/



			nInBufLen -= 8;

			pInBuf += 8;

	

			dest_i=0; /*dest_iָ��dest_buf��һ��λ��*/

		}

	}



	/*��ԭ����*/



	nPlainLen=*pOutBufLen;

	while (nPlainLen)

	{

		if (dest_i<8)

		{

			*(pOutBuf++)=dest_buf[dest_i]^iv_pre_crypt[dest_i];

			dest_i++;

			nPlainLen--;

		}

		else if (dest_i==8)

		{

			/*dest_i==8*/



			/*�ı�ǰһ�����ܿ��ָ��*/

			iv_pre_crypt = iv_cur_crypt;

			iv_cur_crypt = pInBuf; 



			/*�⿪һ���µļ��ܿ�*/



			/*���ǰһ������(��dest_buf[]��)*/

			for (j=0; j<8; j++)

				dest_buf[j]^=pInBuf[j];



			TeaDecryptECB(dest_buf, pKey, dest_buf);



			/*��ȡ����ʱ������ǰһ������(iv_pre_crypt)*/

		

			nInBufLen -= 8;

			pInBuf += 8;

	

			dest_i=0; /*dest_iָ��dest_buf��һ��λ��*/

		}

	}



	/*У��Zero*/

	for (i=1;i<=ZERO_LEN;)

	{

		if (dest_i<8)

		{

			if(dest_buf[dest_i]^iv_pre_crypt[dest_i]) return 0;

			dest_i++;

			i++;

		}

		else if (dest_i==8)

		{

			/*�ı�ǰһ�����ܿ��ָ��*/

			iv_pre_crypt = iv_cur_crypt;

			iv_cur_crypt = pInBuf; 



			/*�⿪һ���µļ��ܿ�*/



			/*���ǰһ������(��dest_buf[]��)*/

			for (j=0; j<8; j++)

				dest_buf[j]^=pInBuf[j];



			TeaDecryptECB(dest_buf, pKey, dest_buf);



			/*��ȡ����ʱ������ǰһ������(iv_pre_crypt)*/



			nInBufLen -= 8;

			pInBuf += 8;

	

			dest_i=0; /*dest_iָ��dest_buf��һ��λ��*/

		}

	

	}



	return 1;

}



