.PHONY: all clean

RM = rm
RMF = -rf

CC = gcc
CFLAGS = -g
LIBFLAGS =


EXE1 = producer
EXE2 = consumer

SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, %.o,$(notdir ${SRC})) 

all: $(EXE1) $(EXE2)

${EXE1}:producer.o
	$(CC) producer.o -o $@ $(LIBFLAGS)

${EXE2}:consumer.o
	$(CC) consumer.o -o $@ $(LIBFLAGS)
	
%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(RMF) $(EXE1) $(EXE2) *.o