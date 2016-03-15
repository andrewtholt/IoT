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

#include "helper.h"


#define PORT    "9091" /* Port to listen on */
#define BACKLOG     10  /* Passed to listen() */

bool verbose;
/* Signal handler to reap zombie processes */

static void wait_for_child(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
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
                                if(verbose) {
                                    fprintf(stderr,"Already Knowm\n");
                                }
                                Writeline(newsock,(void *)"ERROR:KNOWN\n",12);
                        } else if(!identified && (strcmp(p1,"NODENAME"))) {
                            // If the nodename is not set don't allow me to set anything else
                                if(verbose) {
                                    fprintf(stderr,"Who are you?\n");
                                }
                                Writeline(newsock,(void *)"ERROR:WHO\n",10);
                        } else if(!identified && (!strcmp(p1,"NODENAME"))) {
                            strncpy(nodename,p2,strlen(p2)); // fix this min(strlen(p2),sizeof(nodename))
                            // If nodename not set, and I'm trying to set it then OK.
                            //
                            // Check if nodename is know to me.
                            // If not send ERROR:UNKNOWN and disconnect
                            // If known load config and send OK
                            //
                            identified=true;
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
    close(newsock);
}

int main(int argc,char *argv[]) {
    bool verbose=false;

    int sock;
    char port[6];

    struct sigaction sa;
    struct addrinfo hints, *res;
    int reuseaddr = 1; /* True */

    int opt;

    (void) strncpy(port,PORT,sizeof(port));

    while((opt=getopt(argc,argv,"p:vh?"))!=-1) {
        switch(opt) {
            case 'h':
                printf("\nHelp\n\n");
                exit(0);
                break;
            case 'v':
                verbose=true;
                break;
            case 'p':
                (void) strncpy(port,optarg,sizeof(port));
                break;
            default:
                break;
        }
    }

    if(verbose) {
        printf("\n\tSettings\n\n");
        printf("port:    %4s\n",port);
    }

    /* Get the address info */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(NULL, port, &hints, &res) != 0) {
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

