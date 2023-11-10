CC = gcc
CFLAGS = -Wall
LIBS = -ljansson

all: quiz

quiz: play.c quiz.c fetch.c parse.c
	$(CC) $(CFLAGS) -o quiz play.c quiz.c fetch.c parse.c $(LIBS)

run:
	./quiz

clean:
	rm -f quiz
