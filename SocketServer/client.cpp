#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "client.h"
#include "errors.h"

clientInstance::clientInstance() {
    verbose=false;
    identified=false;
}

void clientInstance::setVerbose(bool flag) {
    verbose=flag;
}

bool clientInstance::getVerbose() {
    return verbose;
}

int clientInstance::cmdSet(char *name, char *value) {
    int rc=0;

    if(identified) {  
    } else {    // if not identified then the only thing I am allowed
                // to set is NODENAME.
        if(!strcmp(name,"NODENAME")) {
            rc=OK;
            strncpy(nodeName, name,sizeof(nodeName));
        } else {
            rc = REDIS | NOTCONNECTED;
        }
    }
    return rc;
}



