/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

/**
 * server 主要进行请求的分发，接收到新的请求就把请求分发给route，
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>

#include "toxcore/tox.h"
#include "utils/utils.c"

#include "route.h"


#define BOOTSTRAP_ADDRESS "42.96.195.88"
#define BOOTSTRAP_PORT 33445
#define BOOTSTRAP_KEY "7F613A23C9EA5AC200264EB727429F39931A86C39B67FC14D9ECA4EBE0D37F25"
/*
#define BOOTSTRAP_ADDRESS "109.169.46.133"
#define BOOTSTRAP_PORT 33445
#define BOOTSTRAP_KEY "7F31BFC93B8E4016A902144D0B110C3EA97CB7D43F1C4D21BCAE998A7C838821"
*/
#define MY_NAME "BurstLink"

void process_req(int sock);

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void *SigCatcher(int n)
{
  wait3(NULL,WNOHANG,NULL);
}

void friend_request(Tox *messenger, const uint8_t *public_key, const uint8_t *data, uint16_t length, void *userdata) {
}

void friend_message(Tox *messenger, int32_t friendnumber, const uint8_t *string, uint16_t length, void *userdata) {
}

void write_file(Tox *messenger, int32_t friendnumber, uint8_t filenumber,const uint8_t *data, uint16_t length, void *userdata) {
}

void file_print_control(Tox *messenger, int friendnumber, uint8_t send_receive, uint8_t filenumber, uint8_t control_type, const uint8_t *data, uint16_t length, void *userdata) {
}

void file_request_accept(Tox *messenger, int friendnumber, uint8_t filenumber, uint64_t filesize,const uint8_t *filename, uint16_t filename_length, void *userdata) {
}

static Tox *init_tox(int ipv4)
{
    /* Init core */
    int ipv6 = !ipv4;
    Tox *m = tox_new(ipv6);

    /*
    * TOX_ENABLE_IPV6_DEFAULT is always 1.
    * Checking it is redundant, this *should* be doing ipv4 fallback
    */
    if (ipv6 && m == NULL) {
        fprintf(stderr, "IPv6 didn't initialize, trying IPv4\n");
        m = tox_new(0);
    }

    if (ipv4)
        fprintf(stderr, "Forcing IPv4 connection\n");

    if (m == NULL)
        return NULL;

    /* Callbacks */
    //tox_callback_connection_status(m, on_connectionchange, NULL);
    //tox_callback_typing_change(m, on_typing_change, NULL);
    tox_callback_friend_request(m, friend_request, NULL);
    tox_callback_friend_message(m, friend_message, NULL);
    //tox_callback_name_change(m, on_nickchange, NULL);
    //tox_callback_user_status(m, on_statuschange, NULL);
    //tox_callback_status_message(m, on_statusmessagechange, NULL);
    //tox_callback_friend_action(m, on_action, NULL);
    //tox_callback_group_invite(m, on_groupinvite, NULL);
    //tox_callback_group_message(m, on_groupmessage, NULL);
    //tox_callback_group_action(m, on_groupaction, NULL);
    //tox_callback_group_namelist_change(m, on_group_namelistchange, NULL);
    tox_callback_file_send_request(m, file_request_accept, NULL);
    tox_callback_file_control(m, file_print_control, NULL);
    tox_callback_file_data(m, write_file, NULL);

    tox_set_name(m, MY_NAME, strlen(MY_NAME)); // Sets the username
    return m;
}

int init_connection(Tox *m)
{
    //uint8_t *pub_key = hex_string_to_bin(BOOTSTRAP_KEY);
    //int res = tox_bootstrap_from_address(m, BOOTSTRAP_ADDRESS, 0, htons(BOOTSTRAP_PORT), pub_key);
    unsigned char *binary_string = hex_string_to_bin("7F613A23C9EA5AC200264EB727429F39931A86C39B67FC14D9ECA4EBE0D37F25");
    int res = tox_bootstrap_from_address(m, "42.96.195.88", 0, htons(33445), binary_string);
    if (!res) {
        printf("Failed to convert into an IP address. Exiting...\n");
        exit(1);
    }
    printf("%d\n",res);
}

int main(int argc, char *argv[])
{
    Tox *my_tox = init_tox(0);
    init_connection(my_tox);
    
    time_t timestamp0 = time(NULL);
    int on = 0;
    while (1) {
        if (on == 0) {
            if (tox_isconnected(my_tox)) {
                printf("connected to DHT\n");
                on = 1;
            } else {
                time_t timestamp1 = time(NULL);
                if (timestamp0 + 10 < timestamp1) {
                    timestamp0 = timestamp1;
                    init_connection(my_tox);
                }
                
            }
        }
        tox_do(my_tox);
    }
    
    int sockfd, newsockfd, portno,pid;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    signal(SIGCHLD,SIG_IGN);
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
       error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
            sizeof(serv_addr)) < 0) 
            error("ERROR on binding");
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    while(1){
        newsockfd = accept(sockfd, 
              (struct sockaddr *) &cli_addr, 
              &clilen);
        if (newsockfd < 0) 
            error("ERROR on accept");
        pid = fork();
        if(pid < 0){
            error("fork error");
        }
        if(pid == 0){
            process_req(newsockfd);
            close(newsockfd);
            exit(0);
        }else{
            close(newsockfd);
        }
        tox_do(my_tox);
    }
    close(sockfd);
    return 0; 
}


void process_req(int sock){
    int n;
    char buffer[1024];
        
    bzero(buffer,1024);
    n = read(sock,buffer,1023);
    if (n < 0) error("ERROR reading from socket");
    printf("Message Received:\n %s\n",buffer);
    route(buffer);
    printf("Run here");
    n = write(sock,"I got your message",18);
    if (n < 0) error("ERROR writing to socket");
}