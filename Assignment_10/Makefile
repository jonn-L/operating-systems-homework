CC = gcc 
CFLAGS = -Wall -Wextra -pedantic 

TARGET = server
FOLDER =  
SOURCE = lib/tcp-accept.c lib/tcp-listen.c lib/tcp.h lib/tcp-read.c lib/tcp-write.c chlng.c chlng.h player.c player.h server.c

all: 
	${CC} ${CFLAGS} ${SOURCE} -o ${TARGET}

run: 
	./${TARGET}

clean:
	rm -rf ${TARGET}

.PHONY: all run clean 