#include <hiredis.h>
#include <stdlib.h>
#include <mosquitto.h>

#define REDIS_DB 1
#define SQLITE_DB 2

#ifdef __cplusplus

class clientInstance {
    bool identified;
    bool verbose;
    bool locked;
    bool brokerConnected;

    char inBuffer[255];
    char outBuffer[255];
    char nodeName[255];

    redisContext *data=(redisContext *)NULL;

    int connectToRedis(char *ip, int port);
    int connectToSQLITE();
    redisReply *redisCmd(char *cmd);

    int cmdSet( char *name, char *value);
    void cmdGet( char *name, char *value);
    int cmdSub(char *name);
    int cmdPub(char *name,char *value);
    int cmdClear(char *name);
    int cmdDump();
    int cmdConnect();  // connect to broker, MQTT in this case
    int cmdLock();
    void doClearAll();

    public:
        clientInstance();
        void display(void);

        void setVerbose(bool);
        bool getVerbose();

        /*
        int cmdSet( char *name, char *value);
        char *cmdGet( char *name, int *error);
        */

        int cmdParser(char *cmd, char *result);

        int cmdExit();

        int connectToDB(int dbType, char *ip, int port);

        bool connected();


        ~clientInstance();
};

#endif
