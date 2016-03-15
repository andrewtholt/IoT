/* 
 *  A forked server
 *  by Martin Broadhurst (www.martinbroadhurst.com)
 */

#include <stdio.h>
#include <string.h> /* memset() */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdlib.h>

#include <hiredis.h>

#include "helper.h"

#include "globals.h"


#define PORT    "9090" /* Port to listen on */
#define BACKLOG     10  /* Passed to listen() */
// 
// Error Codes


#define OK 0
#define REDIS 0x80
#define CONNECTFAIL 0x01
#define ALREADYCONNECTED 0x02
#define UNKNOWN 0x04

globalSettings globals;
/*
struct {
    bool verbose;
    int redisPort;
    char redisIp[32];
} globals;
*/

/* Signal handler to reap zombie processes */

static void wait_for_child(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void errorMessage(int rc,char *msg) {

    switch(rc) {
        case OK:
            strcpy(msg,"OK\n");
            break;
        case (REDIS | CONNECTFAIL):
            strcpy(msg,"ERROR:REDIS CONNECTFAIL\n");
            break;
        case (REDIS | ALREADYCONNECTED):
            strcpy(msg,"WARNING:REDIS CONNECTED\n");
            break;
        case (REDIS | UNKNOWN):
            strcpy(msg,"WARNING:REDIS CLIENT_UNKNOWN\n");
            break;
        default:
            strcpy(msg,"ERROR:UNKNOWN\n");
            break;
    }
}

// IN: Redis server ip address
// IN: Redis Port number 
// IN: Client name
// OUT: Result code
// RETURN: redis context.
//
redisContext *connectToRedis(char *ip, int port, char *name, int *rc) {
    redisContext *c;
    redisReply *r;
    redisReply *r1;

    c = redisConnect(ip,port);
    if( c!=NULL && c->err) {
        fprintf(stderr,"Error: %s\n", c->errstr);
        *rc = REDIS | CONNECTFAIL;
        return ((redisContext *)NULL);
    }

    r=(redisReply *)redisCommand(c,"PING");
    freeReplyObject(r);

    r = (redisReply *)redisCommand(c,"HMGET %s connected",name);
    printf("Reply=%s\n", r->element[0]->str);

    if( !r->element[0]->str ) {  // if name not found
        *rc=REDIS|UNKNOWN;
        c=((redisContext *)NULL);
        freeReplyObject(r);
        return(c);
    }

    if( !strcmp(r->element[0]->str,"false")) { // Not connected
        r1 = (redisReply *)redisCommand(c,"HSET %s connected true",name);
        freeReplyObject(r1);
        *rc=0;
    } else {
        *rc = REDIS | CONNECTFAIL;
        redisFree(c);
        c=((redisContext *)NULL);
    }
    freeReplyObject(r);

    *rc=0;
    return(c);
}
/*
 * ATH:  This is where the real work is done.
 */

void handle(int newsock) {
    /* recv(), send(), close() */

    bool runFlag=true;
    bool identified = false;
    int rc=0;
    char buffer[255];

    char *ptr;
    char *p1=(char *)NULL;
    char *p2=(char *)NULL;
    char nodename[255];
    redisContext *data;
    int error=0;


    /*
     * Get name of client (this will be used to create the client name).
     *
     * If unknown create an entry in db.
     * If known mark as connected.
     *
     * check for any messages TO the client and deliver them.
     *
     * wait for destination and message.
     */
    while(runFlag) {
        rc=Readline(newsock,(void *)buffer,sizeof(buffer));

        if( rc == 0) {
            runFlag=false;
        }

        if( rc > 0 ) {
            if(strlen(buffer) > 0) {
                printf("Buffer:>%s<\n",buffer);
                if(buffer[0] == '^') {
                    printf("\tCommand\n");

                    ptr = strtok(buffer," \r\n");
                    if(!strcmp(ptr,"^exit")) {
                        runFlag=false;
                    } else if(!strcmp(ptr,"^set")) {
                        p1=strtok(NULL," ");
                        p2=strtok(NULL," \r\n");

                        if(identified && (!strcmp(p1,"NODENAME"))) {
                            // If the nodename is set, don't allow me to change it.
                            if(globals.getVerbose()) {
                                fprintf(stderr,"Already Knowm\n");
                            }
                            Writeline(newsock,(void *)"ERROR:KNOWN\n",12);
                        } else if(!identified && (strcmp(p1,"NODENAME"))) {
                            // If the nodename is not set don't allow me to set anything else
                            if(globals.getVerbose()) {
                                fprintf(stderr,"Who are you?\n");
                            }
                            Writeline(newsock,(void *)"ERROR:WHO\n",10);
                        } else if(!identified && (!strcmp(p1,"NODENAME"))) {
                            data=connectToRedis(globals.getRedisIP(),globals.getRedisPort(), p2, &error);

                            if( data != (redisContext *) NULL ) {
                                globals.setNodeName(p2);
                                identified=true;
                                Writeline(newsock,(void *)"OK\n",3);
                            } else {
                                char m[255];

                                errorMessage(rc,m);
                                Writeline(newsock,(void *)m,strlen(m));
                                identified=false;
                            }
                            // If nodename not set, and I'm trying to set it then OK.
                            //
                            // Check if nodename is know to me.
                            // If not send ERROR:UNKNOWN and disconnect
                            // If known load config and send OK
                            //
                        } else if(identified) {
                            // Nodename set.
                            //
                            sprintf(buffer,"HSET %s %s %s\n", nodename,p1,p2);
                            Writeline(newsock,buffer,strlen(buffer));
                        }
                    }

                } else {
                }
            }
        }
    }
    redisCommand(data,"HSET %s connected false", globals.getNodeName());
    close(newsock);
    exit(0);
}

int main(int argc,char *argv[]) {
    bool verbose=false;

    int sock;

    struct sigaction sa;
    struct addrinfo hints, *res;
    int reuseaddr = 1; /* True */

    int opt;


    while((opt=getopt(argc,argv,"P:p:R:vh?"))!=-1) {
        switch(opt) {
            case 'h':
                printf("\nHelp\n\n");
                exit(0);
                break;
            case 'v':
                globals.setVerbose(true);
                break;
            case 'p':  // Port that I will listen to.
                globals.setPort(optarg);
                break;
            case 'P':   // Port redis listens on
                globals.setRedisPort(atoi(optarg));
                break;
            case 'R':   // Set Redis IP address.
                globals.setRedisIP( optarg );
                break;
            default:
                break;
        }
    }

    if(globals.getVerbose()) {
        printf("\n\tSettings\n\n");
        globals.display();
    }

    /* Get the address info */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(NULL, globals.getPort(), &hints, &res) != 0) {
        perror("getaddrinfo");
        return 1;
    }

    /* Create the socket */
    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (sock == -1) {
        perror("socket");
        return 1;
    }

    /* Enable the socket to reuse the address */

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1) {
        perror("setsockopt");
        return 1;
    }

    /* Bind to the address */

    if (bind(sock, res->ai_addr, res->ai_addrlen) == -1) {
        perror("bind");
        return 1;
    }

    /* Listen */

    if (listen(sock, BACKLOG) == -1) {
        perror("listen");
        return 1;
    }

    freeaddrinfo(res);

    /* Set up the signal handler */
    sa.sa_handler = wait_for_child;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    /* Main loop */
    while (1) {
        struct sockaddr_in their_addr;
        size_t size = sizeof(struct sockaddr_in);
        int newsock = accept(sock, (struct sockaddr*)&their_addr, &size);
        int pid;

        if (newsock == -1) {
            perror("accept");
            return 0;
        }

        printf("Got a connection from %s on port %d\n", inet_ntoa(their_addr.sin_addr), htons(their_addr.sin_port));

        pid = fork();
        if (pid == 0) {
            /* In child process */
            close(sock);
            handle(newsock);
            return 0;
        }
        else {
            /* Parent process */
            if (pid == -1) {
                perror("fork");
                return 1;
            }
            else {
                close(newsock);
            }
        }
    }

    close(sock);

    return 0;
}

