#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sqlite3.h>
#include <mosquitto.h>
#include <mqueue.h>

#include "client.h"
#include "errors.h"
#include "globals.h"
#include "helper.h"

extern globalSettings globals;
extern struct map *head;

int mySocket;

// 
// Search for longname in list,
// Return shortname, or NULL if not found.
//
char *listMatchLong(char *name) {
    struct map *ptr;
    char *result=(char *)NULL;

    ptr=head;

    do {
        if(!strcmp( ptr->longName, name)) {
            return(ptr->shortName);
        }

        ptr=ptr->next;
    } while( ptr );

    return((char *)NULL);
}

void connect_callback(struct mosquitto *mosq, void *obj, int result) {
    printf("Connected %d!\n\n",result);
    globals.display();
    printf("==============\n");
}

void disconnect_callback(struct mosquitto *mosq, void *obj, int result) {
    printf("Disconnected %d!\n\n",result);
}

void subscribe_callback( struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos) {
    int rc=-1;
    extern mqd_t mq;

    printf("Subscribe\n");
    printf("mySocket:%d\n", mySocket);
    printf("mq      :%d\n", mq);

    rc=mq_send(mq, "TESTING", 7, 1);
    if( rc < 0) {
        fprintf(stderr,"mq_send\n");
        perror("\t");
    }
}

// TODO: Investigate sending received message to a message queue.
// Main loop will then poll the message fd, and the socket.
//
void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message) {
    char *name;
    char scratch[255];

    name = listMatchLong( message->topic );

    sprintf(scratch,"^set %s %s\n", (char *)name, (char *)message->payload);
    Writeline(mySocket, (void *)scratch, strlen(scratch));

    // Alternative to the above Writeline
    /*
    rc=mq_send(mq, scratch, strlen(scratch), 1);
    if( rc < 0) {
        fprintf(stderr,"mq_send\n");
        perror("\t");
    }
    */
}

struct mosquitto *clientInstance::getMQTTHandle() {
    return mosq;
}
// Return the long name in the pointer passed in from the caller.
//
int clientInstance::getMap(char *shortName, char *longName) {
    char sqlCmd[255];
    int rc=OK;

    sqlite3_stmt *sqlRes;
    char *zErrMsg = (char *)NULL ; // Pointer to error message, don't forget to free the memory.

    if(!globalsDatabase) {
        char scratch[255];

        sprintf(scratch,"%s/globals.db",globals.getDbPath());
        rc = sqlite3_open(scratch,&globalsDatabase);
        if( rc == 0) {
            printf("Globals db opened OK.\n");
        } else {
            printf("Globals db open FAILED.\n");
        }

    }
    sprintf(sqlCmd,"select path from mqtt_map where name = '%s'", shortName);

    if(verbose) {
        printf("SQL:>%s<\n", sqlCmd);
    }

    rc = sqlite3_prepare(globalsDatabase, sqlCmd,-1, &sqlRes,0);

    if(rc == 0) {
        rc = sqlite3_step(sqlRes);
        if(rc == SQLITE_ROW) {
            printf("Long version is >%s<\n", sqlite3_column_text(sqlRes,0));
            strcpy(longName,(char *)sqlite3_column_text(sqlRes,0));

            rc=DATABASE_OK;
        }
    } else {
        fprintf(stderr,"SQL Error %s\n", sqlite3_errmsg(globalsDatabase));
        //        sqlite3_free(zErrMsg);
        rc=DATABASE|GENERALERROR;
    }
    return(rc);
}

clientInstance::clientInstance(char *path, int s, pid_t iam) {

    verbose=true;
    identified=false;
    locked=false;
    brokerConnected=false;
    mySocket = s;
    myPid=iam;

    printf("Socket passed in %d\n",s);
    db = (struct sqlite3 *)NULL;

    mosq = (struct mosquitto *)NULL;

    globalsDatabase = (struct sqlite3 *)NULL;

    strcpy(dbPath,path);

    globals.display();

}

clientInstance::~clientInstance() {
    printf("DESD\n\n");
}

void clientInstance::setVerbose(bool flag) {
    verbose=flag;
}

bool clientInstance::getVerbose() {
    return verbose;
}


// This function is idempotent, i.e.
// It can be called multiple times without ill effect,
//
int clientInstance::connectToSQLITE() {
    int rc=OK;
    char dbName[255];


    // TODO need to be able to add a path to database(s)
    // Open the global settings, this should be protected by the db file
    // being RO.
    //
    // Now open the client specific db.
    //

    if( (sqlite3 *)NULL == db) {
        strncpy(dbName,nodeName,sizeof(dbName));
        strncat(dbName,".db",  sizeof(dbName));
        rc = sqlite3_open(dbName, &db);
    } else {
        rc=OK;
    }

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
    bool force=false;

    if(verbose) {
        fprintf(stderr,"Disconnecting\n");
    }

    if(mosq) {
        rc=mosquitto_disconnect(mosq);

        if(MOSQ_ERR_SUCCESS == rc ) {
            force = false;
        } else {
            force = true;
        }

        rc=mosquitto_loop_stop( mosq, force );
    }

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

    rc = sqlite3_prepare(db, cmdBuffer,-1, &res,0);
    if(rc == 0 ) {
        rc = OK;
    } else {
    }

    rc = sqlite3_step(res);
    if(rc == SQLITE_ROW) {
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

    } else {
        // if not identified then the only thing I am allowed to do is set NODENAME.
        // 
        if(!strcmp(name,"NODENAME")) {
            rc=OK;
            strncpy(nodeName, value,sizeof(nodeName));
            connectToSQLITE();

            sprintf(cmdBuffer, "create table %s_variables ( id integer primary key autoincrement, name varchar unique, value varchar);", nodeName);
            printf("DEBUG:%s\n",cmdBuffer);
            rc = sqlite3_exec(db, cmdBuffer, NULL, 0, &zErrMsg);
            if( 0 == rc) {
                identified=true;
                // 
                // Ok I'm identified, now connect to the MQTT
                // server defined in the database.
                //
                // TODO Add config variable 'AUTOCONNECT' if set, then
                // connect, otherwise need an explicit connect command.
                //
                rc=cmdConnect();
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
    int mosqRc=0;
    int qos=1; // At least once.
    bool retain=true;

    char longName[255];

    rc = getMap( name, (char *)&longName );

    if(DATABASE_OK == rc) {
        rc=mosquitto_publish( mosq, &mosqRc, longName, strlen(value)+1, (void *)value, qos, retain);


    } else {
    }


    return rc;
}

int clientInstance::cmdSub(char *name) {
    int rc = OK;
    
    int mosqRc=0;
    int qos=1; // At least once.
    char longName[255];

    rc = getMap( name, (char *)&longName );
    if(DATABASE_OK == rc) {
        rc=mosquitto_subscribe(mosq,&mosqRc,longName,qos);
    } else {
    }

    return rc;
}

int clientInstance::cmdDump() {

    int rc = OK;

    rc = PARSER|NOTIMPLEMENTED;
    return rc;
}


int clientInstance::cmdConnect() {

    char cmdBuffer[255];
    char address[32];
    char port[6];
    int portNum=0;
    int keepalive = 60;
    int mosqRc=-1;

    int rc = 0x100;

//    struct mosquitto *mosq = NULL;

    bool clean_session = false; // TODO What?

    mosquitto_lib_init();
    mosq = mosquitto_new( nodeName, false, (void *)&verbose);

    if(mosq) {
        mosquitto_connect_callback_set(mosq, connect_callback);
        mosquitto_disconnect_callback_set(mosq, disconnect_callback);
        mosquitto_message_callback_set(mosq, message_callback);
        mosquitto_subscribe_callback_set(mosq, subscribe_callback);

        rc = mosquitto_connect(mosq, globals.getMQTTAddress(), globals.getMQTTPort(), 60);
        switch(rc) {
            case MOSQ_ERR_SUCCESS:
//                mosquitto_subscribe(mosq, NULL, "#", 0);
                globals.setMQTTConnected(true);
//                mosquitto_loop_start( mosq );
                rc =  MQTT_OK;
                break;
            case MOSQ_ERR_INVAL:
                rc = MQTT|INVALID_PARAMS;
                break;
            case MOSQ_ERR_ERRNO:
                rc = MQTT|CHECK_ERRNO;
                break;
            default:
                rc=GENERALERROR;
                break;
        }

    }

    return rc;
}

void clientInstance::doClearAll() {
    char cmdBuffer[256];

    sprintf(cmdBuffer,"delete from %s_variables", nodeName);
    printf("DEBUG:%s\n", cmdBuffer);
    // 
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
        case PARSER_OK:
        case MQTT_OK:
        case DATABASE_OK:
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




