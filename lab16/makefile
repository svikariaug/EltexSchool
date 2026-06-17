
CC = gcc
CFLAGS = -Wall -Wextra -pthread
RM = rm -f

TASK1_DIR = task1
TASK2_DIR = task2
TASK3_DIR = task3
TASK4_DIR = task4

TASK1_TARGETS = $(TASK1_DIR)/inet_dgram_client \
                $(TASK1_DIR)/inet_dgram_server \
                $(TASK1_DIR)/inet_stream_client \
                $(TASK1_DIR)/inet_stream_server \
                $(TASK1_DIR)/local_dgram_client \
                $(TASK1_DIR)/local_dgram_server \
                $(TASK1_DIR)/local_stream_client \
                $(TASK1_DIR)/local_stream_server

TASK2_TARGETS = $(TASK2_DIR)/time_client \
                $(TASK2_DIR)/time_server_multiprotocol \
                $(TASK2_DIR)/time_server_queue \
                $(TASK2_DIR)/time_server_thread_per_client \
                $(TASK2_DIR)/time_server_thread_pool

TASK3_TARGETS = $(TASK3_DIR)/broadcast_client \
                $(TASK3_DIR)/broadcast_server \
                $(TASK3_DIR)/multicast_client \
                $(TASK3_DIR)/multicast_server

TASK4_TARGETS = $(TASK4_DIR)/raw_client_eth_ip_udp \
                $(TASK4_DIR)/raw_client_ip_udp \
                $(TASK4_DIR)/raw_client_udp \
                $(TASK4_DIR)/udp_modify_server

all: task1 task2 task3 task4

task1: $(TASK1_TARGETS)
task2: $(TASK2_TARGETS)
task3: $(TASK3_TARGETS)
task4: $(TASK4_TARGETS)

$(TASK1_DIR)/inet_dgram_client: $(TASK1_DIR)/inet_dgram_client.c
	$(CC) $(CFLAGS) -o $@ $<

$(TASK1_DIR)/inet_dgram_server: $(TASK1_DIR)/inet_dgram_server.c
	$(CC) $(CFLAGS) -o $@ $<

$(TASK1_DIR)/inet_stream_client: $(TASK1_DIR)/inet_stream_client.c
	$(CC) $(CFLAGS) -o $@ $<

$(TASK1_DIR)/inet_stream_server: $(TASK1_DIR)/inet_stream_server.c
	$(CC) $(CFLAGS) -o $@ $<

$(TASK1_DIR)/local_dgram_client: $(TASK1_DIR)/local_dgram_client.c
	$(CC) $(CFLAGS) -o $@ $<

$(TASK1_DIR)/local_dgram_server: $(TASK1_DIR)/local_dgram_server.c
	$(CC) $(CFLAGS) -o $@ $<

$(TASK1_DIR)/local_stream_client: $(TASK1_DIR)/local_stream_client.c
	$(CC) $(CFLAGS) -o $@ $<

$(TASK1_DIR)/local_stream_server: $(TASK1_DIR)/local_stream_server.c
	$(CC) $(CFLAGS) -o $@ $<

$(TASK2_DIR)/time_client: $(TASK2_DIR)/time_client.c
	$(CC) $(CFLAGS) -o $@ $<

$(TASK2_DIR)/time_server_multiprotocol: $(TASK2_DIR)/time_server_multiprotocol.c
	$(CC) $(CFLAGS) -o $@ $<

$(TASK2_DIR)/time_server_queue: $(TASK2_DIR)/time_server_queue.c
	$(CC) $(CFLAGS) -o $@ $<

$(TASK2_DIR)/time_server_thread_per_client: $(TASK2_DIR)/time_server_thread_per_client.c
	$(CC) $(CFLAGS) -o $@ $<

$(TASK2_DIR)/time_server_thread_pool: $(TASK2_DIR)/time_server_thread_pool.c
	$(CC) $(CFLAGS) -o $@ $<

$(TASK3_DIR)/broadcast_client: $(TASK3_DIR)/broadcast_client.c
	$(CC) $(CFLAGS) -o $@ $<

$(TASK3_DIR)/broadcast_server: $(TASK3_DIR)/broadcast_server.c
	$(CC) $(CFLAGS) -o $@ $<

$(TASK3_DIR)/multicast_client: $(TASK3_DIR)/multicast_client.c
	$(CC) $(CFLAGS) -o $@ $<

$(TASK3_DIR)/multicast_server: $(TASK3_DIR)/multicast_server.c
	$(CC) $(CFLAGS) -o $@ $<

$(TASK4_DIR)/raw_client_eth_ip_udp: $(TASK4_DIR)/raw_client_eth_ip_udp.c
	$(CC) $(CFLAGS) -o $@ $<

$(TASK4_DIR)/raw_client_ip_udp: $(TASK4_DIR)/raw_client_ip_udp.c
	$(CC) $(CFLAGS) -o $@ $<

$(TASK4_DIR)/raw_client_udp: $(TASK4_DIR)/raw_client_udp.c
	$(CC) $(CFLAGS) -o $@ $<

$(TASK4_DIR)/udp_modify_server: $(TASK4_DIR)/udp_modify_server.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	$(RM) $(TASK1_TARGETS) $(TASK2_TARGETS) $(TASK3_TARGETS) $(TASK4_TARGETS)
	$(RM) $(TASK1_DIR)/*.o $(TASK2_DIR)/*.o $(TASK3_DIR)/*.o $(TASK4_DIR)/*.o
	# Удаляем файлы сокетов для AF_LOCAL
	$(RM) /tmp/local_dgram_socket /tmp/local_stream_socket 2>/dev/null || true

clean-task1:
	$(RM) $(TASK1_TARGETS)

clean-task2:
	$(RM) $(TASK2_TARGETS)

clean-task3:
	$(RM) $(TASK3_TARGETS)

clean-task4:
	$(RM) $(TASK4_TARGETS)

# Task 1 запуск
run-task1-udp-server:
	./$(TASK1_DIR)/inet_dgram_server

run-task1-udp-client:
	./$(TASK1_DIR)/inet_dgram_client

run-task1-tcp-server:
	./$(TASK1_DIR)/inet_stream_server

run-task1-tcp-client:
	./$(TASK1_DIR)/inet_stream_client

run-task1-local-dgram-server:
	./$(TASK1_DIR)/local_dgram_server

run-task1-local-dgram-client:
	./$(TASK1_DIR)/local_dgram_client

run-task1-local-stream-server:
	./$(TASK1_DIR)/local_stream_server

run-task1-local-stream-client:
	./$(TASK1_DIR)/local_stream_client

# Task 2 запуск
run-task2-thread-server:
	./$(TASK2_DIR)/time_server_thread_per_client

run-task2-pool-server:
	./$(TASK2_DIR)/time_server_thread_pool

run-task2-queue-server:
	./$(TASK2_DIR)/time_server_queue

run-task2-multiprotocol-server:
	./$(TASK2_DIR)/time_server_multiprotocol

run-task2-client:
	./$(TASK2_DIR)/time_client

# Task 3 запуск
run-task3-broadcast-server:
	./$(TASK3_DIR)/broadcast_server

run-task3-broadcast-client:
	./$(TASK3_DIR)/broadcast_client

run-task3-multicast-server:
	./$(TASK3_DIR)/multicast_server

run-task3-multicast-client:
	./$(TASK3_DIR)/multicast_client

# Task 4 запуск
run-task4-udp-server:
	sudo ./$(TASK4_DIR)/udp_modify_server

run-task4-raw-client-udp:
	sudo ./$(TASK4_DIR)/raw_client_udp

run-task4-raw-client-ip-udp:
	sudo ./$(TASK4_DIR)/raw_client_ip_udp

run-task4-raw-client-eth-ip-udp:
	sudo ./$(TASK4_DIR)/raw_client_eth_ip_udp

.PHONY: all task1 task2 task3 task4 clean clean-task1 clean-task2 clean-task3 clean-task4 help
.PHONY: run-task1-udp-server run-task1-udp-client run-task1-tcp-server run-task1-tcp-client
.PHONY: run-task1-local-dgram-server run-task1-local-dgram-client run-task1-local-stream-server run-task1-local-stream-client
.PHONY: run-task2-thread-server run-task2-pool-server run-task2-queue-server run-task2-multiprotocol-server run-task2-client
.PHONY: run-task3-broadcast-server run-task3-broadcast-client run-task3-multicast-server run-task3-multicast-client
.PHONY: run-task4-udp-server run-task4-raw-client-udp run-task4-raw-client-ip-udp run-task4-raw-client-eth-ip-udp