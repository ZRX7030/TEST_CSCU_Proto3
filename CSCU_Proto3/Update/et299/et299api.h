 /*
[]=========================================================================[]

Copyright(C) 2011, Feitian Technologies Co., Ltd.
All rights reserved.
 
FILE:
	FT_et299api.h
	
DESC:
	Key interface and defines 
		
[]=========================================================================[]
*/

#ifndef  __FT_ET299_HEADER_H
#define  __FT_ET299_HEADER_H



//Return Value Status Definition
#define	  ET_SUCCESS			0x00		//Function execute success
#define   ET_ACCESS_DENY		0x01		//Accessing refuse or Popedom not enough
#define   ET_COMMUNICATIONS_ERROR	0x02		//Communicate error or Can't open device	
#define   ET_INVALID_PARAMETER		0x03		//Invalid argument or Parameter error
#define   ET_NOT_SET_PID		0x04		//Not intercalate PIN
#define   ET_UNIT_NOT_FOUND		0x05		//Open appoint device error
#define   ET_HARD_ERROR			0x06		//Hardware error
#define   ET_UNKNOWN			0x07		//Universality error
#define	  ET_PIN_ERR_MASK		0x0F		//Validate PIN Mask 
#define	  ET_PIN_ERR_MAX		0xFF            //Validate PIN Maximum number of remaining, if error NO. return ET_PIN_MAX mean never locked

//Add new error number
#define	  ET_KEY_TYPE			0X10		//Key type error
#define	  ET_LICENSE_TYPE		0X11		//License typer error
#define	  ET_LICENSE_UNINIT		0x12		//Authorization unit is not enabled the factory
#define   ET_NO_LICENSE			0x13		//Authorization failed
#define   ET_UNKNOWN_UPDATE_FILE	0x14		//Unknown file format upgrade
#define	  ET_INVALID_DATA_LEN		0x15		//Illegal data length
#define   ET_NO_TDES_KEY		0x16		//TDES key invalid
#define   ET_INVALID_UPDATE_FILE	0x17		//Upgrade file invalid
#define	  ET_UNSUPPORTED		0x18		//Unsupported features
#define	  ET_ADJUST_TIMER		0x19		//Adjust the system clock failure
#define	  ET_KEY_INDEX			0X1A		//The key index error
#define	  ET_KEY_ZERO			0X1B		//Zero keys for the system

//Linux add new error code
#define   ET_TOOMUCHTHREAD		0x1C		//Process to open the lock with a number of threads > 100

//Constant definitions
#define ET299_MEMORY_SIZE		6*1024		//ET299 data buffer

//Factory settings
#define CONST_PID   	                "FFFFFFFF"

struct SYSTEMTIME {
    unsigned short wYear; 
    unsigned short wMonth; 
    unsigned short wDayOfWeek; 
    unsigned short wDay; 
    unsigned short wHour; 
    unsigned short wMinute; 
    unsigned short wSecond; 
    unsigned short wMilliseconds; 
} ; 

//Find key ,PID input and find out the number of key
unsigned int 	et_FindToken(unsigned char* pid/*[in]*/,int * count);

//Open the specified PID , index input and anony user access to the state
unsigned int 	et_OpenToken(int* pHandle,unsigned char* pid,int index);

//Close token
unsigned int 	et_CloseToken(int hHandle);

//Read Et299 data buffer
unsigned int 	et_Read(int hHandle,unsigned short offset,int Len,unsigned char* pucReadBuf);

//Write data to key buffer
unsigned int 	et_Write(int hHandle,unsigned short offset,int Len,unsigned char* pucWriteBuf);

//Produce product identification PID
unsigned int 	et_GenPID(int hHandle,int SeedLen,unsigned char* pucSeed,unsigned char* pid);

//Generate random numbers
unsigned int 	et_GenRandom(int hHandle,unsigned char* pucRandBuf);

//Generate new SOPIN number
unsigned int 	et_GenSOPIN(int hHandle,int SeedLen,unsigned char* pucSeed, unsigned char* pucNewSoPIN);

//Ordinary users to reset the  PIN code
unsigned int 	et_ResetPIN(int hHandle,unsigned char* pucSoPIN);

//Set the key,Compatible with the ET99, only set HMAC-MD5 key, the key 32-tyte key length
unsigned int 	et_SetKey(int hHandle,int Keyid, unsigned char* pucKeyBuf);

//Extended of the original Et99 interface,  you can set TDES key, and 3DES key 16-tyte key length
unsigned int 	et_SetKeyEx(int hHandle,int keyType,int Keyid, unsigned char* keyBuf);
//Select the key type 
#define ET_KEY_HMAC_MD5				0	//select HMAC_MD5 type
#define ET_KEY_TDES				1	//select TDES type

//HMAC-MD5 operation, enter the key index, data and length, Output 16-tyte digest
unsigned int 	et_HMAC_MD5(int hHandle,int keyID,int textLen,unsigned char* pucText,unsigned char *digest);

//Vaildate PIN 
unsigned int 	et_Verify(int hHandle,int Flags,unsigned char* pucPIN);

//define pin Flags
#define ET_VERIFY_USERPIN			0
#define ET_VERIFY_SOPIN				1

//Change userPIN, input older PIN and new PIN
unsigned int 	et_ChangeUserPIN(int hHandle,unsigned char* pucOldPIN,unsigned char* pucNewPIN);

//Reset security status
unsigned int 	et_ResetSecurityState(int hHandle);//Need to reset

//Back to the hardware serial number, factory burned into the hardware serial number of bytes
unsigned int 	et_GetSN(int hHandle,unsigned char* pucSN/*8bytes*/);

//Set SoPIN retries
unsigned int 	et_SetupToken(int hHandle,unsigned char bSoPINRetries,unsigned char bUserPINRetries,unsigned char bUserReadOnly,unsigned char bBack);

#define ET_USER_WRITE_READ				0	//Read and write
#define ET_USER_READ_ONLY				1	//Only read

//Turn on led
unsigned int 	et_TurnOnLED(int hHandle);

//turn off led
unsigned int 	et_TurnOffLED(int hHandle);

//Get custom hardware serial number
unsigned int 	et_GetSNEx(int hHandle, unsigned char* pucSN,int* len);

//Set custom hardware serial number
unsigned int 	et_SetSNEx(int hHandle,unsigned char* pucSNEx,int len);

//Generate the RSA key pair, e=65537 and return,RSA key pair can only be returned at this time, can not read other time
unsigned int 	et_RSAGenKey(int hHandle,int keyLen,unsigned char* pBufferRSA_N, unsigned char* pBufferRSA_D);

#define RSA_KEY_LEN_1024	1024	//the RSA 1024-tyte key length
	
//set RSA ,be used for RSA operation
unsigned int 	et_RSASetKey(int hHandle,int keyLen,unsigned char* pBufferRSA_N, unsigned char* pBufferRSA_D);
//Private RSA key set  of remote upgrade, be used for remote update
unsigned int 	et_RSASetUpdateKey(int hHandle,int keyLen,unsigned char* pBufferRSA_N, unsigned char* pBufferRSA_D);

#define ET_RSA_PKCS1_PADDING		1	//PKCS#1 fill in
#define ET_RSA_NO_PADDING		2	//No fill in
//RSA public key encryption
unsigned int 	et_RSAPublicEncrypt(int hHandle,unsigned char* buffer,int *len,int nFillMode);

//RSA public key decryption
unsigned int 	et_RSAPublicDecrypt(int hHandle,unsigned char* buffer,int *len,int nFillMode);

//RSA private key to encryption
unsigned int   et_RSAPrivateEncrypt(int hHandle,unsigned char* buffer,int* len,int nFillMode);

//RSA private key to decryption
unsigned int 	et_RSAPrivateDecrypt(int hHandle,unsigned char* buffer,int* len,int nFillMode);

//3DES encryption
unsigned int 	et_TDesEncrypt(int hHandle,int keyIndex,unsigned char* buffer,int len);

//3DES decryption
unsigned int 	et_TDesDecrypt(int hHandle, int keyIndex,unsigned char* buffer,int len);

//Set the time authorized
unsigned int 	et_SetTimeLicense(int hHandle, int unitIndex,int timeMode,unsigned char* newTime);

#define ET_EXPIRE_DATE			0	//Deadline authorized
#define ET_TIME_MINUTE			1	//Time authorized(unit:minute)
#define ET_TIME_DAY			2	//authorized the day(unit:data)

//Time to get licensing
unsigned int 	et_GetTimeLicense(int hHandle, int unitIndex,int *timeMode,unsigned char* newTime);

//Enable time accredit
unsigned int 	et_StartTimeUnit (int hHandle, int unitIndex);

//Stop time accredit
unsigned int 	et_StopTimeUnit (int hHandle, int unitIndex);

//Adjust key system time
unsigned int 	et_AdjustTimer(int hHandle, unsigned char* newTime);

//Authorized account sub-set
unsigned int 	et_SetCountLicense(int hHandle, int unitIndex,unsigned int count, unsigned char type);

//Authorized access to meter times
unsigned int 	et_GetCountLicense(int hHandle, int unitIndex,unsigned int *count,unsigned char *type);

//Testing the legality of the upgrade file
unsigned int 	et_CheckUpdateFile(int hHandle,unsigned char* updateBuffer,int len);

//Remote upgrade
unsigned int 	et_RemoteUpdate (int hHandle,unsigned char* updateBuffer,int len);

//Generated by the authority to upgrade a remote file
unsigned int 	et_GenRemoteUpdateFile (int hHandle, unsigned char* updateBuffer, int len);

//Get version
unsigned int 	et_GetVersion (int hHandle, unsigned short* version);

/*{{{ MD5 HMAC Wrapper functions.*/
//Software achieve
unsigned int 
MD5_HMAC(unsigned char * pucText,       	 /* pointer to data stream        */
		 unsigned int   ulText_Len,     /* length of data stream         */
		 unsigned char * pucKey,         /* pointer to authentication key */
		 unsigned int   ulKey_Len,      /* length of authentication key  */
		 unsigned char * pucToenKey,     /* inner hashed key and  outer hashed key */
		 unsigned char * pucDigest );    /* caller digest to be filled in */
/*}}}*/



#endif	//__FT_ET299_HEADER_H

