.PHONY:clean 
BINS=chatclient chatserv
CC=g++
CC_FALGS=-Wall -O -O2 -g
all:$(BINS)
%.o:%.cpp
	$(CC) $(CC_FALGS) $^ -c -o $@
clean:
	rm core *.o $(BINS)
