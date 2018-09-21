#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include "../ET299_data.h"

int main()
{
	void *Error;
    void *(*start)();
	void *handle =dlopen("/mnt/nandflash/lib/libet299.so",RTLD_NOW);
	Error = dlerror();
	if(Error)
	{
		printf("get libcan.so fail \n");
		return 0;
	//获取失败
	}else
	{				
		start=dlsym(handle,"Start");
		if(Error)
		{
			return 0;													
		}
					
	}
	ET_Lock data;
	memset(&data,0,sizeof(data));
	int n= (int)start(&data);
	if(n<0)
	{
		printf("Read et299 fail\n");
	}
	if(data.flag==0)
	{
		printf("verify fail\n");
	}else 
	{
		printf("Verify successufl\n");
		printf("data:%s\n",data.data);
	}
	return 0;
}	
		
