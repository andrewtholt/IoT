#include <hiredis.h>

#define REDIS_DB 1

#ifdef __cplusplus

class clientInstance {
    bool identified;
    bool verbose;

    char inBuffer[255];
    char outBuffer[255];
    char nodeName[255];

    redisContext *data;

    int connectToRedis(char *ip, int port);
    redisReply *redisCmd(char *cmd);

    public:
        clientInstance();
        void display(void);

        void setVerbose(bool);
        bool getVerbose();

        int cmdSet( char *name, char *value);
        char *cmdGet( char *name, int *error);

        int cmdExit();

        int connectToDB(int dbType, char *ip, int port);

        bool connected();

        ~clientInstance();
};

#endif
