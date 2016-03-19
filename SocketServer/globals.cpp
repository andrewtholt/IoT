#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "globals.h"

globalSettings::globalSettings() {
    verbose=false;
    redisPort=6379;

    strcpy(redisIp,"127.0.0.1");
    strcpy(port,"9090");

    namePtr=(char *)NULL;
}

void globalSettings::display() {

    printf("Verbose      : ");
    if(verbose) {
        printf("True\n");
    } else {
        printf("False\n");
    }
    printf("Port         : %s\n",port);

    printf("Redis Server : %s\n",redisIp);
    printf("Redis Port   : %d\n",redisPort);

    printf("\n");
}

void globalSettings::setVerbose(bool flag) {
    verbose=flag;
}

bool globalSettings::getVerbose() {
    return verbose;
}

void globalSettings::setRedisIP(char *ip) {
    strcpy(redisIp,ip);
}

char *globalSettings::getRedisIP() {
    return(redisIp);
}

void globalSettings::setRedisPort(int p) {
    redisPort=p;
}

int globalSettings::getRedisPort() {
    return redisPort;
}

char *globalSettings::getPort() {
    return port;
}

void globalSettings::setPort(char *p) {
    strncpy(port,p,sizeof(port));
}


