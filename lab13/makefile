CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS_POSIX = -lrt
LDFLAGS_CHAT = -lrt -lpthread -lncurses

TARGETS = chat_server chat_client posix_server posix_client sysv_server sysv_client

all: $(TARGETS)

chat_server: chat_server.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS_CHAT)

chat_client: chat_client.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS_CHAT)

posix_server: posix_server.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS_POSIX)

posix_client: posix_client.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS_POSIX)

sysv_server: sysv_server.c
	$(CC) $(CFLAGS) -o $@ $<

sysv_client: sysv_client.c
	$(CC) $(CFLAGS) -o $@ $<

run-chat-server: chat_server
	./chat_server

run-chat-client: chat_client
	./chat_client

run-posix-server: posix_server
	./posix_server

run-posix-client: posix_client
	./posix_client

run-sysv-server: sysv_server
	./sysv_server

run-sysv-client: sysv_client
	./sysv_client

run1: posix_server posix_client
	@./posix_server & \
	sleep 1; \
	./posix_client; \
	pkill -f "./posix_server" 2>/dev/null || true

run2: sysv_server sysv_client
	@./sysv_server & \
	sleep 1; \
	./sysv_client; \
	pkill -f "./sysv_server" 2>/dev/null || true

clean-queues:
	-mq_unlink /chat_server 2>/dev/null || true
	-mq_unlink /server_queue 2>/dev/null || true
	-mq_unlink /client_queue 2>/dev/null || true
	-ipcrm -Q 1234 2>/dev/null || true
	-ipcrm -Q 5678 2>/dev/null || true

clean:
	rm -f $(TARGETS)

distclean: clean clean-queues

.PHONY: all clean clean-queues distclean help \
        run-chat-server run-chat-client \
        run-posix-server run-posix-client \
        run-sysv-server run-sysv-client