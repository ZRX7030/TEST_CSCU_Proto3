PROG = upload
CC=arm-none-linux-gnueabi-gcc

INC_PATH += -I./include/
LIB_PATH += -L./lib/

vpath %.c  ./src

LIBS += -lcurl -lsqlite3 ./lib/libz.a  ./lib/libjson-c.a 
LDFLAGS = 
CFLAGS = -Wall

OBJS += config.o signals.o json_interface.o compress.o http_interface.o main.o database.o upload.o log.o 

$(PROG): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LIB_PATH) $(LIBS)

%.o: %.c
	$(CC) -o $@ -c $< $(CPPFLAGS) $(CFLAGS) $(INC_PATH)

.PHONY : clean distclean

install:
	cp -f ./upload  ../CSCU_A1/FileSystemFiles/nandflash/bin

clean:
	rm -f *.o

distclean: clean
	rm -f $(PROG)



