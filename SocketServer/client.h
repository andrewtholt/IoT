#include <stdlib.h>
#include <mosquitto.h>
#include <sqlite3.h>

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
    char dbPath[255];
    sqlite3 *db;        // Connection to this clients database
    sqlite3 *globalsDatabase; // RO connection to global data.

    int connectToSQLITE();

    struct mosquitto *mosq;

    int cmdSet( char *name, char *value);
    int cmdGet( char *name, char *value);
    int cmdSub(char *name);
    int cmdPub(char *name,char *value);
    int cmdClear(char *name);
    int cmdDump();
    int cmdConnect();  // connect to broker, MQTT in this case
    int cmdLock();
    void doClearAll();
    // 
    // Look up shortname, and return full MQTT path
    int getMap(char *shortName,char *longName);

    public:
        clientInstance(char *path, int s);
        void display(void);

        void setVerbose(bool);
        bool getVerbose();

        int cmdParser(char *cmd, char *result);

        int cmdExit();

        int connectToDB(int dbType, char *ip, int port);

        bool connected();

        ~clientInstance();
};

#endif
