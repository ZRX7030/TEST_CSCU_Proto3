#ifndef __AES_H_
#define __AES_H_
/*
 * aes.h
 *
 * 类描述：AES加密算法
 * 创建: LDM 20150401
 * 修改记录:见aes.cpp
 *
 */

//enum KeySize { Bits128, Bits192, Bits256 };  // key size, in bits, for construtor
#define Bits128	16
#define Bits192	24
#define Bits256	32

	typedef unsigned char   byte;



void Aes(int keySize, unsigned char* keyBytes);
void Cipher(unsigned char* input, unsigned char* output);  // encipher 16-bit input
void InvCipher(unsigned char* input, unsigned char* output);  // decipher 16-bit input

int AesEncrypt(unsigned char* input,unsigned int iDataLen,unsigned char *psPK ,unsigned char* output,int iType=1);
int AesDeEncrypt(unsigned char * input,int len,unsigned char* output,char is_encrypt);

void SetNbNkNr(int keySize);
void AddRoundKey(int round);      //轮密钥加
void SubBytes();                  //S盒字节代换
void InvSubBytes();               //逆S盒字节代换
void ShiftRows();                 //行移位
void InvShiftRows();
void MixColumns();                //列混淆
void InvMixColumns();
unsigned char gfmultby01(unsigned char b);
unsigned char gfmultby02(unsigned char b);
unsigned char gfmultby03(unsigned char b);
unsigned char gfmultby09(unsigned char b);
unsigned char gfmultby0b(unsigned char b);
unsigned char gfmultby0d(unsigned char b);
unsigned char gfmultby0e(unsigned char b);
void KeyExpansion();              //密钥扩展
void SubWord(unsigned char *wd);         //密钥S盒字代换
void RotWord(unsigned char *wd);         //密钥移位

void DataToStr(unsigned char *psData,int iLen,unsigned char *psOut);


#endif
