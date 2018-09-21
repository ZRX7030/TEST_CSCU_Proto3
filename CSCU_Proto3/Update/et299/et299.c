#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "et299api.h"
#include "3DES.h"
#include "ET299_data.h"

#define ET_ADD_THREEDES         1
#define ET_FRE_THREEDES 		0
#define SUCCESS                 0
#define PIN "FFC5EB786A2F0D73"
#define OFFSET 0x150
#ifdef __cplusplus
//extern "C" {
//#endif
int g_hToken = -1;

//产生随机数函数
int et_rand(int len,unsigned char * out)
{
	int i =0;
	srand((unsigned)time(NULL));
	for(i=0;i<len;i++)
	{
			out[i] = rand()%256;
	}
	return 0;		
 }

void Print_Hex(unsigned char *data, int len)
{
	int i=0;
	for(;i<len;i++)
	{
		printf("%02x ",data[i]);
		if((i+1)%16==0)
        {
		    printf("\n");
        }
  	}
}

unsigned int ShowErrInfo(unsigned int retval)
{
	switch(retval)
	{
	case ET_SUCCESS:
		break;
	case ET_ACCESS_DENY:
		printf("Err: access denied, have not enough right!");
		break;
	case ET_COMMUNICATIONS_ERROR:
		printf("Err: have not open the device");
		break;
	case ET_INVALID_PARAMETER:
		printf("Err: invalid parameter!");
		break;
	case ET_NOT_SET_PID:
		printf("Err: have not set PID!");
		break;
	case ET_UNIT_NOT_FOUND:
		printf("Err: open the device fail!");
		break;
    case 0xF0:
		printf("Err: PIN  have been locked!");
		break;
	default :
		{
			if(retval&0xF0)
				printf("Err: PIN is wrong!");
			else
				printf("Err: unknown error occured!");
		}
		break;
	}
	if(retval != 0)
	{
		printf("(0x%x)\n",retval);
		return retval;	
	}
	return 0;
}
/*
int main()
{
	ET_Lock tt;
	memset(&tt,0,sizeof(tt));
	Start(&tt);
	printf("UserInfo:%s\n",tt.data);
	return 0;
}
*/
//int main()
//返回值：大于0（用户信息长度），小于0(失败原因)
//buff 返回用户信息

//extern "C" int Start(ET_Lock * ET_Data);

int Start(ET_Lock * ET_Data)
{
	unsigned int retval = 0;
	int i=0;	
	int count = 0;

    unsigned char BUFF1[32] = {0};//读出BUFF_64[64]的前32字节放入Buff1[32]
    unsigned char BUFF2[32] = {0};//读出BUFF_64[64]的后32字节放入Buff2[32]
    unsigned char OUT_DE_BUFF2[32] = {0};//解密BUFF2的结果存放在OUT_DE_BUFF2
    unsigned char key1[16] = {0};//从 BUFF1提取出key1
    unsigned char key2[16] = {0};//从 OUT_DE_BUFF2取出key2
    unsigned char key3[16] = {0};//从 OUT_DE_BUFF2取出key3


	unsigned int offs = 0;
	unsigned int rLen = 0;

    unsigned char rBuff[6144] = {0};//打印用户信息的数据	
   //***** Please Input  PID *****
	unsigned char pid_test[20] = "ffc5eb78";
	// Find 查找计算机上指定pid的ET299个数。 也就是插了几个u盘

	retval = et_FindToken(pid_test,&count);
	if(ET_SUCCESS != retval)
    {
        printf("Find ERR\n");
		//return ShowErrInfo(retval);
        return -retval;
	}

    printf("Find %d key!!!\n",count);
	 
   	// Open    打开指定PID的硬件, 由index指定打开硬件的索引, index应该小于等于找到的Token数目。打开后进入匿名用户状态。et_OpenToken(ET_HANDLE* hHandle, unsigned char *pid, int index); 
	retval = et_OpenToken(&g_hToken,pid_test,count); //** count : Insert number of the Dongle,Please change..!**
	if(ET_SUCCESS != retval)
	{
		printf("Open ERR\n");
		//return ShowErrInfo(retval);
        return -retval;
	}
    else
    {
        printf("Open NO.%d Success!\n",count);
    }	
	
	//************ VERIFY_TEST (Login Stat) ***********
	// Verify So
	//et_Verify(ET_HANDLE hHandle, int Flags, unsigned char *PIN); 验证密码, 以获得相应的安全状态, 不受安全状态限制, 验证成功以后, 进入相应的安全状态。 
	retval = et_Verify(g_hToken,ET_VERIFY_SOPIN,PIN);
	if(ET_SUCCESS != retval)
	{
		printf("Login S_User ERR..\n");
	//	return ShowErrInfo(retval);
        return -retval;
	}
	else
    {
        printf("Login S_User Success!!!\n");
    }

	// READ..
	//  et_Read(ET_HANDLE hHandle, WORD offset, int Len, unsigned char *pucReadBuf); 从指定的位置, 读取指定长度的数据到指定的BUFFER中。该函数调用需要通过USER PIN或者SO PIN验证，调用后不改变安全状态。
    unsigned char buff[128];
	rLen = 72;
	retval = et_Read(g_hToken,OFFSET,(unsigned short)rLen,buff);
	if(ET_SUCCESS != retval)
	{
		printf("Read Data ERR\n");
	//	return ShowErrInfo(retval);
         return -retval;
	}
	else	
	{
		printf("\n =>> %d (0x%X) byte%s read.\n", rLen, rLen, rLen > 1 ? "s" : "");
	}	
		
    memcpy(BUFF1,buff,32);
    memcpy(BUFF2,buff+32,32);

	for(i = 0;i<16;i++)
	{	
		if(i%2 ==0)
			key1[i] = BUFF1[i];
		else
			key1[i] = BUFF1[31-i];
	}
	

//ocean debug 
	printf("[key]\n");
	Print_Hex(key1,16);
	printf("[key]\n");
	
		//将加密的结果以3des算法解密	
	data_convert(BUFF2,32,OUT_DE_BUFF2,ET_FRE_THREEDES,key1);
       
    memcpy(key2,OUT_DE_BUFF2,16);
    memcpy(key3,OUT_DE_BUFF2+16,16);

//ocean debug 
	printf("[key2]\n");
	Print_Hex(key2,16);
	printf("[key2]\n");
	
//ocean debug 
	printf("[key3]\n");
	Print_Hex(key3,16);
	printf("[key3]\n");
	



    //key1 = key2?
    retval = memcmp(key1,key2,16);
	if(SUCCESS != retval)
	{
		ET_Data->flag=0; //fail
		return -0xffff ; //鉴权失败
	}		
			
	ET_Data->flag = 1;  //鉴权成功

	// igned int retval = 0;        
	// READ.. 身份信息
    rLen =buff[64] + (buff[65]<<8) + (buff[66]<<16) + (buff[67]<<24);
	
	//  et_Read(ET_HANDLE hHandle, WORD offset, int Len, unsigned char *pucReadBuf); 从指定的位置, 读取指定长度的数据到指定的BUFFER中。该函数调用需要通过USER PIN或者SO PIN验证，调用后不改变安全状态。
	retval = et_Read(g_hToken,OFFSET+72,rLen,rBuff);
	if(ET_SUCCESS != retval)
	{
		printf("Read Data ERR\n");
	//	return ShowErrInfo(retval);
        return -retval;
	}
	else	
	{
		printf("\n =>> %d (0x%X) byte%s read.\n", rLen, rLen, rLen > 1 ? "s" : "");
	}

	//解密身份信息
	ET_Data->datalen = data_convert(rBuff,rLen,ET_Data->data,ET_FRE_THREEDES,key3);
	
	//ocean debug
	printf("[Recv]\n");	
	Print_Hex(buff,72);
	Print_Hex(rBuff,rLen);
	printf("\n[Recv]\n");	

	int ilen=buff[68] + (buff[69]<<8) +(buff[70]<<16 ) + (buff[71]<<24 );
	printf("[UserInfo]\n");
	Print_Hex(ET_Data->data,ET_Data->datalen);
	printf("\n[UserInfo]\n");

	// ******** Close *************
	retval = et_CloseToken(g_hToken);
	if(ET_SUCCESS != retval)
	{
		printf("Close ERR\n");
	//	return ShowErrInfo(retval);
	    return -retval;
	}

	return 0;
}

//#ifdef __cplusplus
//}
//#endif
