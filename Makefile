CC     = gcc
CFLAGS = -Wall -Wextra -O3 --std=gnu99 

all: apacheclf2json
clean: 
	$(RM) apacheclf2json parser.c *.o

apacheclf2json: parser.o json.o jv-utils.o jv_unicode.o md5.o base64.o url.o
	$(CC) $(CFLAGS) -o $@ $+

parser.c: parser.rl 
	ragel -G2 parser.rl -o parser.c
