#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "client.h"
#include "errors.h"

clientInstance::clientInstance() {
    verbose=false;
    identified=false;
    locked=false;
}

clientInstance::~clientInstance() {
}

void clientInstance::setVerbose(bool flag) {
    verbose=flag;
}

bool clientInstance::getVerbose() {
    return verbose;
}

int clientInstance::connectToRedis(char *ip,int port) {
    int rc=GENERALERROR;

    data = redisConnect(ip,port);
    if( data != (redisContext *)NULL ) {
        if(data->err != 0) {
            if(verbose) {
                fprintf(stderr,"Error: %s\n", data->errstr);
            }
            rc=REDIS|CONNECTFAIL;
        } else {
            redisReply *r;
            r=(redisReply *)redisCommand(data,"PING");
            // TODO Check for correct response.
            freeReplyObject(r);
            rc=OK;
        }
    }

    return rc;
}

// Connect to database, currently redis only.
//
int clientInstance::connectToDB( int dbType,char *ip, int port ) {
    int rc;

    switch(dbType) {
        case REDIS_DB:
            rc=connectToRedis(ip,port);
            break;
        default:
            rc=UNKNOWNDBTYPE;
            break;
    }

    if( rc != 0 ) {
        return rc;
    }

    return rc;
}


redisReply *clientInstance::redisCmd(char *cmd) {
    redisReply *r;

    r = (redisReply *)redisCommand(data,cmd);

    return r;
}

int clientInstance::cmdExit() {
    redisReply *r;
    char cmdBuffer[255];
    int rc=0;

    sprintf(cmdBuffer,"HSET %s connected false",nodeName);
    r=redisCmd(cmdBuffer);
    freeReplyObject(r);
    rc=CLIENTEXIT;

    return rc;
}

void clientInstance::cmdGet(char *name,char *value) {
    char cmdBuffer[255];
    redisReply *r;

    sprintf(cmdBuffer,"HGET %s %s", nodeName,name);
    r=redisCmd(cmdBuffer);

    sprintf(value,"OK:%s\n",r->str);
}

int clientInstance::cmdSet(char *name, char *value) {
    int rc=OK;
    char cmdBuffer[255];
    redisReply *r;

    if(identified) {  // If known can't set NODENAME

        if(!locked) {
            sprintf(cmdBuffer,"HSET %s %s %s",nodeName,name,value);
            r=redisCmd(cmdBuffer);
        // 
        // TODO check for errors
        //
            freeReplyObject(r);
        } else {
            rc=OK;
        }

    } else {    // if not identified then the only thing I am allowed
        // to set is NODENAME.
        if(!strcmp(name,"NODENAME")) {
            rc=OK;
            strncpy(nodeName, value,sizeof(nodeName));

            sprintf(cmdBuffer,"HGET %s connected",nodeName);
            r=redisCmd(cmdBuffer);

            if( !strcmp(r->str,"false")) {
                // Not already connected.
                freeReplyObject(r);
                sprintf(cmdBuffer,"HSET %s connected true",nodeName);
                r=redisCmd(cmdBuffer);
                freeReplyObject(r);
                identified=true;
            } else {
            }
        } else {
            rc = REDIS | NOTCONNECTED;
        }
    }
    return rc;
}

void clientInstance::lock() {
    locked=true;
}

int clientInstance::cmdParser(char *cmd,char *reply) {

    int rc=OK;
    char *c;
    char *p1;
    char *p2;

    if( cmd[0] != '^' ) {
        rc = PARSER|NOTACMD;
    } else {
        c = (char *)strtok( cmd, " \r\n");
        if (!strcmp(c,"^lock")) {
            lock();
            strcpy(reply,"OK\n");
            rc=OK;
        } else if(!strcmp(c,"^exit")) {
            rc=cmdExit();
        } else if(!strcmp(c,"^get")) {
            p1 = (char *)strtok( NULL, " \r\n");
            cmdGet( p1,reply);
            rc=OK;

        } else if(!strcmp(c,"^set")) {
            p1 = (char *)strtok( NULL, " \r\n");
            p2 = (char *)strtok( NULL, " \r\n");

            rc = cmdSet(p1,p2);

            if( rc == OK ) {
                sprintf(reply,"OK\n");
            }
        } else {
            rc = PARSER|UNKNOWN;
        }

    }
    return rc;
}

int clientInstance::cmdParser(char *cmd) {
}



