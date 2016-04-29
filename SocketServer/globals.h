
#ifdef __cplusplus
class globalSettings {
    bool verbose;
    char dbPath[32];
    char port[8];
    char *namePtr;

    public:

        globalSettings();
        void display(void);

        void setVerbose(bool flag);
        bool getVerbose();

        void setDbPath(char *path);
        char *getDbPath();

        void setPort(char *p);
        char *getPort();
};
#endif
