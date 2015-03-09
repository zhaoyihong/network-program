.PHONY:clean 
BINS=echo_ser echo_client
CC=g++
CC_FALGS=-Wall -O -O2
all:$(BINS)
echo_ser:sysutils.o echo_ser.o
echo_client:sysutils.o echo_client.o
%.o:%.cpp
	$(CC) $(CC_FALGS) $^ -c -o $@
clean:
	rm *.o $(BINS)
