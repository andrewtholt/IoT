/*
 *  A forked server
 * based on work  by Martin Broadhurst (www.martinbroadhurst.com)
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
#include <sqlite3.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <mqueue.h>

#include "helper.h"

#include "globals.h"
#include "errors.h"
#include "client.h"


#define PORT    "9090" /* Port to listen on */
#define BACKLOG     10  /* Passed to listen() */
//
// Error Codes

/*
#define OK 0
#define REDIS 0x80
#define CONNECTFAIL 0x01
#define ALREADYCONNECTED 0x02
#define UNKNOWN 0x04
*/

globalSettings globals;

/* Signal handler to reap zombie processes */

static void wait_for_child(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void errorMessage(int rc,char *msg) {

    switch(rc) {
        case OK:
            strcpy(msg,"OK\n");
            break;
        case (DATABASE | CONNECTFAIL):
            strcpy(msg,"ERROR:DATABASE CONNECT FAIL\n");
            break;
        case (DATABASE | ALREADYCONNECTED):
            strcpy(msg,"WARNING:DATABASE CONNECTED\n");
            break;
        case (DATABASE | UNKNOWN):
            strcpy(msg,"WARNING:DATABASE CLIENT_UNKNOWN\n");
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
/*
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
 globals.display();

 *rc=0;
 return(c);
 }
 */
/*
 * ATH:  This is where the real work is done.
 */
mqd_t mq;

void handleConnection(int newsock) {
    /* recv(), send(), close() */

    bool runFlag=true;
    bool identified = false;
    int rc=0;
    char buffer[255];

    char mqName[255];

    char outBuffer[255];
    char nodeName[255];

    char *ptr;
    char *p1=(char *)NULL;
    char *p2=(char *)NULL;
    int error=0;
    pid_t myPid=0;

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 1024;
    attr.mq_curmsgs = 0;

    struct pollfd pfds[2];

    // Get pid ...
    myPid=getpid();
    // 
    // .. use pid to construct posix message path.

    sprintf(mqName,"/fs%08d", myPid);
    //
    mq = mq_open(mqName, O_CREAT | O_RDONLY, 0644, &attr);
    if( mq < 0 ) {
        fprintf(stderr,"FATAL_ERROR:mq_open failed\n");
        perror("handleConnection");
        runFlag = false;
    }

    // 
    // Create a message queue and pass reference into client.
    //
    clientInstance client(globals.getDbPath(),newsock, myPid);
    globals.display();
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

        pfds[0].fd = newsock;
        pfds[0].events=POLLIN;

        pfds[1].fd = mq;
        pfds[1].events=POLLIN;

        memset( buffer, (int) 0, sizeof(buffer));
        memset( outBuffer, (int) 0, sizeof(outBuffer));
        // 
        // use poll on socket and message queue handle.
        //
        poll(pfds,2,-1);

        rc=Readline(newsock,(void *)buffer,sizeof(buffer));

        if (pfds[0].revents == POLLIN ) {

            if( rc == 0) {
                // Client disconnected
                runFlag=false;
                client.cmdExit();

            }

            if( rc > 0 ) {
                if(strlen(buffer) > 0) {
                    int err;
                    printf("Buffer:>%s<\n",buffer);

                    err = client.cmdParser(buffer,outBuffer);
                    Writeline(newsock,outBuffer,strlen(outBuffer));
                    if (CLIENTEXIT == err ) {
                        runFlag=false;
                    }
                }
            }
        }
    }
    // Cleanup and exit.
    mq_unlink(mqName);
    close(newsock);
    exit(0);
}

void usage(char * name) {
    printf("\nUsage:%s -h|-v|-p <listen> -P <database path>\n", name);
    printf("\t-h\t\tHelp.\n");
    printf("\t-v\t\tVerbose.\n");
    printf("\t-p <listen>\tPort that this service listens on.\n");
    printf("\t-P\t\tPath to global database.\n");
    printf("\n");
    printf("Default behaviour is equivalent to:\n");
    printf("\t%s -p 9090 -R 127.0.0.1 -P /tmp\n",name);
    printf("\n");

}

struct map *head;

int main(int argc,char *argv[]) {
    bool verbose=false;

    int sock;

    struct sigaction sa;
    struct addrinfo hints, *res;
    int reuseaddr = 1; /* True */

    int opt;
    char scratch[255];

    sqlite3 *globalDb;
    sqlite3_stmt *sqlRes;
    int rc;


    while((opt=getopt(argc,argv,"P:p:R:vh?"))!=-1) {
        switch(opt) {
            case 'h':
                usage(argv[0]);
                exit(0);
                break;
            case 'v':
                globals.setVerbose(true);
                break;
            case 'p':  // Port that I will listen to.
                globals.setPort(optarg);
                break;
            case 'P':   // Path to databse files
                globals.setDbPath(optarg);
                break;
            default:
                break;
        }
    }
    // 
    // Load settings form globals table.
    // TODO Load short name maps.
    // 
    sprintf(scratch,"%s/globals.db",globals.getDbPath());
    rc = sqlite3_open(scratch,&globalDb);
    if (rc !=0) {
        fprintf(stderr,"No globals database at %s defaults in place.\n",scratch);
    } else {
        sprintf(scratch,"select address,port from mqtt_settings limit 1");

        if(globals.getVerbose()) {
            printf("sql >%s<\n",scratch);
        }
        rc = sqlite3_prepare(globalDb,scratch,-1,&sqlRes,0);
        if (rc == 0 ) {
            rc = sqlite3_step(sqlRes);
            if(rc == SQLITE_ROW) {
                globals.setMQTTAddress( (char *)sqlite3_column_text(sqlRes,0) );
                globals.setMQTTPort( (char *)sqlite3_column_text(sqlRes,1) );
            }
        }
    }
    // Read maps into a linked list, this will be search 'brute force'
    // by the client.
    // TODO SIGHUP to parent process forces the destruction of 
    // the current list, and a new one to be created.  Connected children 
    // will be unaffected until they disconnect and re-connect.
    //
    // Finished with globals database for now, so close it.
    // 
    // Globals should be all sorted by now, so lock
    // them down.
    //

    sprintf(scratch,"select name,path from mqtt_map;");
    rc = sqlite3_prepare(globalDb,scratch,-1,&sqlRes,0);

    if (rc == 0 ) {
        struct map *ptr;
        struct map *tmp;
        do {
            rc = sqlite3_step(sqlRes);
            if(rc == SQLITE_ROW) {
                printf("Short:%s\n",(char *)sqlite3_column_text(sqlRes,0));
                printf("Long :%s\n",(char *)sqlite3_column_text(sqlRes,1));

                ptr=(struct map *)malloc( sizeof(struct map));
                if( (struct map *)NULL == ptr) {
                    fprintf(stderr, "Fatal error, failed to allocate space for map\n\n");
                    exit(128);
                }

                ptr->next=(struct map *)NULL;
                if( (struct map *)NULL == head) {
                    head = ptr;
                } else {
                    tmp = head;
                    head = ptr;
                    ptr->next = tmp;
                }

                strncpy(ptr->shortName,(char *)sqlite3_column_text(sqlRes,0),sizeof(ptr->shortName));
                strncpy(ptr->longName,(char *)sqlite3_column_text(sqlRes,1),sizeof(ptr->longName));
            }
        } while(rc == SQLITE_ROW);
    }

    globals.lock();
    if(globals.getVerbose()) {
        printf("\n\tSettings\n\n");
        globals.display();
    }

    sprintf(scratch,"%s/globals.db",globals.getDbPath() );
    if(globals.getVerbose()) {
        printf("Globals db %s\n", scratch);
    }
    //
    // Get the address info.
    //
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
        socklen_t size = sizeof(struct sockaddr_in);
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
            handleConnection(newsock);
            return(0) ;
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

