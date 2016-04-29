#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sqlite3.h>

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


int clientInstance::connectToSQLITE() {
    int rc=OK;
    char dbName[255];


    // TODO need to be able to add a path to database(s)
    // Open the global settings, this should be protected by the db file
    // being RO.
    //
    // Now open the client specific db.
    //
    strncpy(dbName,nodeName,sizeof(dbName));
    strncat(dbName,".db",  sizeof(dbName));
    rc = sqlite3_open(dbName, &db);

    return rc;
}
// Connect to database, currently sqlite.
//
int clientInstance::connectToDB( int dbType,char *ip, int port ) {
    int rc;

    switch(dbType) {
        case REDIS_DB:
            break;
        case SQLITE_DB:
            rc=connectToSQLITE();
            break;
        default:
            break;
    }

    if( rc != 0 ) {
        return rc;
    }

    return rc;
}

int clientInstance::cmdExit() {
    char cmdBuffer[255];
    int rc=0;
    char *zErrMsg = 0;

    sprintf(cmdBuffer,"drop table if exists %s_variables",nodeName);
    rc = sqlite3_exec(db, cmdBuffer, NULL, 0, &zErrMsg);
    if ( rc == 0) {
        rc=CLIENTEXIT;
    }
    return rc;
}

int clientInstance::cmdGet(char *name,char *value) {
    char cmdBuffer[255];
    char *zErrMsg = 0;
    int rc;
    sqlite3_stmt *res;

    sprintf(cmdBuffer,"select value from %s_variables where name='%s'", nodeName, name );
    printf("DEBUG:%s\n",cmdBuffer);
//    rc = sqlite3_exec(db, cmdBuffer, NULL, 0, &zErrMsg);

    rc = sqlite3_prepare(db, cmdBuffer,-1, &res,0);
    if(rc == 0 ) {
        rc = OK;
    } else {
    }

    rc = sqlite3_step(res);
    if(rc == SQLITE_ROW) {
//        printf("DATA:%s\n",sqlite3_column_text(res,0));
        sprintf(value,"DATA:%s\n",sqlite3_column_text(res,0));
    } else {
        fprintf(stderr,"ERROR %s\n",sqlite3_errmsg(db));
    }

    sqlite3_finalize(res);

    return rc;
}

int clientInstance::cmdSet(char *name, char *value) {
    int rc=OK;
    char cmdBuffer[255];
    char *zErrMsg = 0;

    if(identified) {  // If known can't set NODENAME

        if(!locked) {
            sprintf(cmdBuffer,"replace into %s_variables (name,value) values ('%s','%s')", nodeName,name,value);
            printf("DEBUG:%s\n",cmdBuffer);
            rc = sqlite3_exec(db, cmdBuffer, NULL, 0, &zErrMsg);
        // 
        // TODO check for errors
        //
        } else {
            rc=OK;
        }

    } else {    // if not identified then the only thing I am allowed
        // to set is NODENAME.
        if(!strcmp(name,"NODENAME")) {
            rc=OK;
            strncpy(nodeName, value,sizeof(nodeName));
            connectToSQLITE();

            sprintf(cmdBuffer, "create table %s_variables ( id integer primary key autoincrement, name varchar unique, value varchar);", nodeName);
            printf("DEBUG:%s\n",cmdBuffer);
            rc = sqlite3_exec(db, cmdBuffer, NULL, 0, &zErrMsg);
            if( 0 == rc) {
                identified=true;
            }
        } else {
            rc = DATABASE | NOTCONNECTED;
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

    char cmdBuffer[255];
//    redisReply *r;
    char address[32];
    char port[6];
    int portNum=0;
    int keepalive = 60;
    int mosqRc=-1;

    int rc = 0x100;
    struct mosquitto *mosq = NULL;

    bool clean_session = false; // TODO What?

// Get mqtt setting from global database.
//
    mosquitto_lib_init();

    mosq = mosquitto_new(nodeName,clean_session,NULL);

    if(!mosq) {
        printf("MQTT ERROR\n");
    } else {
        mosqRc = mosquitto_connect(mosq,address,portNum,keepalive);
        printf("%s\n",mosquitto_strerror( mosqRc ));

        switch( mosqRc ) {
            case MOSQ_ERR_SUCCESS:
                rc=OK;
                break;
            case MOSQ_ERR_INVAL:
                rc=-1;  // TODO Replace with a meaningful value
                break;
            case MOSQ_ERR_ERRNO:
                rc = -2; // TODO Replace with a meaningful value
                break;
            default:
                rc = -3; // TODO Replace with a meaningful value
                break;
        }
    }

//    rc = PARSER|NOTIMPLEMENTED;
    return rc;
}

int clientInstance::cmdSub(char *name) {
//    redisReply *r;
    int rc = OK;
    char cmdBuffer[255];
    char scratchBuffer[255];

    rc = PARSER|NOTIMPLEMENTED;

    return rc;
}

void clientInstance::doClearAll() {
    char cmdBuffer[255];
//    redisReply *r;

    sprintf(cmdBuffer,"delete from %s_variables", nodeName);
    printf("DEBUG:%s\n", cmdBuffer);
    // TODO make sure connected flag is still set.
    //
}

int clientInstance::cmdClear(char *name) {
    char cmdBuffer[255];

    int rc = OK;

    if( name == (char *)NULL) {
        doClearAll();
        return rc;
    }

    if (!strcmp(name,"all" )) {
        doClearAll();
    } else {
        sprintf(cmdBuffer,"delete from %s_variables where name='%s'", nodeName,name);
        printf("DEBUG:%s\n", cmdBuffer);
    
    }

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
            if(strlen(reply) == 0) {
                sprintf(reply,"OK\n");
            }
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




