CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS_POSIX = -lrt -lpthread
LDFLAGS_CHAT = -lrt -lpthread -lncurses
LDFLAGS_SYSV = 
TARGETS = chat_server chat_client server_posix client_posix server_sysv client_sysv

all: $(TARGETS)

chat_server: chat_server.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS_CHAT)

chat_client: chat_client.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS_CHAT)

server_posix: server_posix.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS_POSIX)

client_posix: client_posix.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS_POSIX)

server_sysv: server_sysv.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS_SYSV)

client_sysv: client_sysv.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS_SYSV)

run-chat-server: chat_server
	./chat_server

run-chat-client: chat_client
	./chat_client

run-posix-server: server_posix
	./server_posix

run-posix-client: client_posix
	./client_posix

run-sysv-server: server_sysv
	./server_sysv

run-sysv-client: client_sysv
	./client_sysv

run1: server_posix client_posix
	@./server_posix & \
	sleep 1; \
	./client_posix; \
	pkill -f "./server_posix" 2>/dev/null || true

run2: server_sysv client_sysv
	@./server_sysv & \
	sleep 1; \
	./client_sysv; \
	pkill -f "./server_sysv" 2>/dev/null || true

clean-ipc:
	-shm_unlink /chat_shm 2>/dev/null || true
	-shm_unlink /myshm 2>/dev/null || true
	-sem_unlink /chat_mutex 2>/dev/null || true
	-sem_unlink /chat_msg_count 2>/dev/null || true
	-sem_unlink /mysem 2>/dev/null || true
	-ipcrm -M 1234 2>/dev/null || true

clean:
	rm -f $(TARGETS)
	
distclean: clean clean-ipc
.PHONY: all clean clean-ipc distclean run-chat-server run-chat-client run-posix-server run-posix-client run-sysv-server run-sysv-client demo-posix demo-sysv