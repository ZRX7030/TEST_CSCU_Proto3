CC = arm-arago-linux-gnueabi-gcc
et299.so : 3DES.c et299.c 3DES.h ET299_data.h libET299.a 
	$(CC) -fPIC -shared 3DES.c et299.c libET299.a  -o libet299.so


clean :
	rm *.o
