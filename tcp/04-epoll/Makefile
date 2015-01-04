.PHONY:clean 
BINS=echo_ser echo_client
CC=g++
CC_FALGS=-Wall -O -O2
all:$(BINS)
echo_ser:echo_ser.o readline.o
echo_client:echo_client.o readline.o
%.o:%.cpp
	$(CC) $(CC_FALGS) $^ -c -o $@
clean:
	rm *.o $(BINS)
