typedef struct 
{
	unsigned char flag;//0:fail 1:successful
	char data[1024];  //user info
	int datalen;
}ET_Lock;
