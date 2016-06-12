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

    strcpy(hostName,"127.0.0.1");
    strcpy(port,"9090");

    strcpy(mqtt_ip,"127.0.0.1");
    mqtt_port=1883;
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
    printf("Host         : %s\n",hostName);
    printf("Port         : %s\n",port);
    printf("DB Path      : %s\n",dbPath);

    printf("==== MQTT ====\n");
    printf("MQTT IP      : %s\n",mqtt_ip);
    printf("MQTT Port    : %d\n",mqtt_port);

    printf("Locked       : ");
    if(locked) {
        printf("True");
    } else {
        printf("False");
    }

    printf("\n");
    printf("==============\n");
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

char *globalSettings::getHost() {
    return hostName;
}

void globalSettings::setHost(char *name) {
    strncpy(hostName,name,sizeof(hostName));
}

char *globalSettings::getPort() {
    return port;
}

void globalSettings::setPort(char *p) {
    if(!locked) {
        strncpy(port,p,sizeof(port));
    }
}

void globalSettings::setMQTTAddress(char *addr) {
    if(!locked) {
        strncpy(mqtt_ip,addr,sizeof(mqtt_ip));
    }
}

void globalSettings::setMQTTPort(char *port) {
    if(!locked) {
        mqtt_port=atoi(port);
    }
}

char *globalSettings::getMQTTAddress() {
    return(mqtt_ip);
}

int globalSettings::getMQTTPort() {
    return(mqtt_port);
}

