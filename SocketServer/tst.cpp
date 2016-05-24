#include <stdio.h>
#include "globals.h"

int main() {
    int c;

    globalSettings globals;

    globals.display();

    globals.setVerbose(true);
    globals.setDbPath((char *)"/var/tmp/");

    printf("dbPath=%s\n",globals.getDbPath());
    globals.lock();
    globals.display();

}

