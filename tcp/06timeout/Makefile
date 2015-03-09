.PHONY:clean 
BINS=echo_ser echo_client
SER_OBJ=readline.o sysutil.o echo_ser.o
CLI_OBJ=readline.o sysutil.o echo_client.o
CC=g++
CC_FALGS=-Wall -O -g
all:$(BINS)
echo_ser:$(SER_OBJ)
	$(CC) $^  $(CC_FALGS)  -o $@
echo_client:$(CLI_OBJ)
	$(CC) $^  $(CC_FALGS)  -o $@
%.o:%.cpp
	$(CC) $^ $(CC_FALGS) -c -o $@
clean:
	rm $(SER_OBJ)  $(CLI_OBJ) $(BINS)
