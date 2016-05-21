#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "globals.h"

globalSettings::globalSettings() {
    // 
    // Set defaults.
    //
    verbose=false;

    strcpy(dbPath,"/tmp");
    strcpy(port,"9090");

//    namePtr=(char *)NULL;


}

void globalSettings::lock() {
    locked = true;
}

void globalSettings::display() {

    printf("Verbose      : ");
    if(verbose) {
        printf("True\n");
    } else {
        printf("False\n");
    }
    printf("Port         : %s\n",port);
    printf("DB Path      : %s\n",dbPath);
    printf("Locked       : ");
    if(locked) {
        printf("True");
    } else {
        printf("False");
    }


    printf("\n");
}

void globalSettings::setVerbose(bool flag) {
    verbose=flag;
}

bool globalSettings::getVerbose() {
    return verbose;
}

void globalSettings::setDbPath(char *p) {
    if(!locked) {
        strcpy(dbPath,p);
    }
}

char *globalSettings::getDbPath(void) {
    return(dbPath);
}

char *globalSettings::getPort() {
    return port;
}

void globalSettings::setPort(char *p) {
    if(!locked) {
        strncpy(port,p,sizeof(port));
    }
}


