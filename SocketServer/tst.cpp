#include <stdio.h>
#include "globals.h"

int main() {
    int c;

    globalSettings globals;

    globals.display();

    globals.setVerbose(true);
    globals.setRedisIP("192.168.100.22");
    globals.setRedisPort(1234);

    globals.display();

    printf("Test RedisIP %s\n",globals.getRedisIP());
}

