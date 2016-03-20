#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "client.h"
#include "errors.h"

clientInstance::clientInstance() {
    verbose=false;
    identified=false;
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

    return rc;
}

int clientInstance::cmdSet(char *name, char *value) {
    int rc=0;
    char cmdBuffer[255];
    redisReply *r;

    if(identified) {  // If known can't set NODENAME
        sprintf(cmdBuffer,"HSET %s %s %s",nodeName,name,value);
        r=redisCmd(cmdBuffer);
        // 
        // TODO check for errors
        //
        freeReplyObject(r);

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



