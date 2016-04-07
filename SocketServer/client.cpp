#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "client.h"
#include "errors.h"

clientInstance::clientInstance() {
    verbose=false;
    identified=false;
    locked=false;
    brokerConnected=false;
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

int clientInstance::cmdLock() {
    int rc=OK;
    locked=true;
    return rc;
}

int clientInstance::cmdPub(char *name,char *value) {

    int rc = OK;

    rc = PARSER|NOTIMPLEMENTED;
    return rc;
}

int clientInstance::cmdDump() {

    int rc = OK;

    rc = PARSER|NOTIMPLEMENTED;
    return rc;
}

int clientInstance::cmdConnect() {

    int rc = OK;

    // 
    // Check if we want to subscribe to any services,
    //
    // connect to MQTT server
    // if subscribed is not nil or length > 0
    //  start thread to listen for MQTT messages
    // endif
    //
    // Listen on socket, only ^pub is allowed

    rc = PARSER|NOTIMPLEMENTED;
    return rc;
}

int clientInstance::cmdSub(char *name) {
    redisReply *r;
    int rc = OK;
    char cmdBuffer[255];
    char scratchBuffer[255];

    sprintf(cmdBuffer,"HGET %s subscribed",nodeName);
    r=redisCmd(cmdBuffer);

    if( r->type == REDIS_REPLY_NIL ) {
        printf("%s undefined\n",name);
        freeReplyObject(r);

        sprintf(cmdBuffer,"HSET %s subscribed %s",nodeName,name);
        r=redisCmd(cmdBuffer);
    } else {
        printf("%s defined\n",name);
        memset( scratchBuffer,0,(size_t)sizeof(scratchBuffer));
        strncpy( scratchBuffer, r->str, (size_t) sizeof(scratchBuffer));
        freeReplyObject(r);
        strcat( scratchBuffer, ":" );
        strcat( scratchBuffer, name );
        sprintf(cmdBuffer,"HSET %s subscribed %s",nodeName,scratchBuffer);
        printf("cmdBuffer is %s\n",cmdBuffer);
        r=redisCmd(cmdBuffer);
    }

//    rc = PARSER|NOTIMPLEMENTED;

    return rc;
}

void clientInstance::doClearAll() {
    char cmdBuffer[255];
    redisReply *r;

    sprintf(cmdBuffer,"DEL %s", nodeName);
    r=redisCmd(cmdBuffer);
    freeReplyObject(r);

    sprintf(cmdBuffer,"HSET %s connected true",nodeName);
    r=redisCmd(cmdBuffer);
    freeReplyObject(r);
}

int clientInstance::cmdClear(char *name) {
    char cmdBuffer[255];
    redisReply *r;
    int rc = OK;

    if( name == (char *)NULL) {
        doClearAll();
        return rc;
    }

    if (!strcmp(name,"all" )) {
        doClearAll();
    } else {
        sprintf(cmdBuffer,"HDEL %s %s",nodeName,name);
        r=redisCmd(cmdBuffer);
        freeReplyObject(r);
    }

    // rc = PARSER|NOTIMPLEMENTED;

    // DEL <name>
    // HSET <name> connected true
    // 
    return rc;
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
            rc=cmdLock();
        } else if(!strcmp(c,"^clear")) {
            p1 = (char *)strtok( NULL, " \r\n");
            rc=cmdClear(p1);
        } else if(!strcmp(c,"^exit")) {
            rc=cmdExit();
        } else if(!strcmp(c,"^get")) {
            p1 = (char *)strtok( NULL, " \r\n");
            cmdGet( p1,reply);
            rc=OK;
        } else if(!strcmp(c,"^sub")) {
            p1 = (char *)strtok( NULL, " \r\n");
            rc = cmdSub(p1);
        } else if(!strcmp(c,"^pub")) {
            p1 = (char *)strtok( NULL, " \r\n");
            p2 = (char *)strtok( NULL, " \r\n");

            rc = cmdPub(p1,p2);
        } else if(!strcmp(c,"^connect")) {
            rc = cmdConnect();
        } else if(!strcmp(c,"^dump")) {
            rc = cmdDump();
        } else if(!strcmp(c,"^set")) {
            p1 = (char *)strtok( NULL, " \r\n");
            p2 = (char *)strtok( NULL, " \r\n");

            rc = cmdSet(p1,p2);
        } else {
            rc = PARSER|UNKNOWN;
        }

    }
    switch(rc) {
        case OK:
            sprintf(reply,"OK\n");
            break;
        case (PARSER|UNKNOWN):
            sprintf(reply,"PARSER:UNKNOWN\n");
            break;
        case PARSER|NOTIMPLEMENTED:
            sprintf(reply,"PARSER:UNIMPLEMENTED\n");
            break;
        case CLIENTEXIT:
            sprintf(reply,"OK\n");
            break;
        default:
            sprintf(reply,"PARSER:0x%02x\n", rc);
            break;
    }
    return rc;
}




