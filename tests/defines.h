#ifndef _DEFINES_H
#define _DEFINES_H

#define SOCK_BUF_SIZE 1024
#define UUID_LENGTH 36
#define CMD_LENGTH 1
#define MESSAGE_LENGTH_BYTE 2
#define MY_MESSAGE_LENGTH UUID_LENGTH + CMD_LENGTH + MESSAGE_LENGTH_BYTE + SOCK_BUF_SIZE
#define BOOTSTRAP_ADDRESS "42.96.195.88"
#define BOOTSTRAP_PORT 33445
#define BOOTSTRAP_KEY "7F613A23C9EA5AC200264EB727429F39931A86C39B67FC14D9ECA4EBE0D37F25"
#define MAX_MSG_CACHE 128

#endif