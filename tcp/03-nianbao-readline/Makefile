.PHONY:clean 
BINS=libreadline.so  echo_ser echo_client
CC=g++
CC_FALGS=-Wall -O -O2 -lreadline -L.
all:$(BINS)
echo_ser:echo_ser.cpp
	$(CC) $^  $(CC_FALGS)  -o $@
echo_client:echo_client.cpp
	$(CC) $^  $(CC_FALGS)  -o $@
libreadline.so:readline.cpp
	$(CC) -fPIC -shared $^ -o $@
clean:
	rm *.o $(BINS)
