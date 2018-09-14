LIB_DIR=../lib
CC=gcc

all: teste1 teste2

teste1: teste1.c $(LIB_DIR)/libcthread.a
	$(CC) -m32 -g -o teste1 teste1.c -L$(LIB_DIR) -lcthread -Wall
	
	
teste2: teste2.c $(LIB_DIR)/libcthread.a
	 $(CC) -m32 -g -o teste2 teste2.c -L$(LIB_DIR) -lcthread -Wall

clean:
	 rm teste1 teste2
