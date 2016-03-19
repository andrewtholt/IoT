
#ifdef __cplusplus
class globalSettings {
    bool verbose;
    int redisPort;
    char redisIp[32];
    char port[8];
    char *namePtr;

    public:

        globalSettings();
        void display(void);

        void setVerbose(bool flag);
        bool getVerbose();

        void setRedisIP(char *ip);
        char *getRedisIP();

        void setRedisPort(int p);
        int getRedisPort();

        void setPort(char *p);
        char *getPort();
};
#endif
