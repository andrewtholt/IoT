#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
class globalSettings {
    bool verbose;
    char dbPath[32];
    char hostName[255];
    char port[8];
//    char *namePtr;
    bool locked=false;
    // MQTT settings, loaded from database.
    //
    char mqtt_ip[17]; // IP Address
    int mqtt_port;

    public:

        globalSettings();
        void display(void);
        void lock(void);

        void setVerbose(bool flag);
        bool getVerbose();

        void setDbPath(char *path);
        char *getDbPath();

        void setPort(char *p);
        char *getPort();

        void setHost(char *p);
        char *getHost();

        void setMQTTAddress(char *addr);
        void setMQTTPort(char *addr);

        char *getMQTTAddress();
        int getMQTTPort();
};
#endif
