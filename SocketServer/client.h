
#ifdef __cplusplus

class clientInstance {
    bool identified;
    bool verbose;

    char inBuffer[255];
    char outBuffer[255];
    char nodeName[255];

    public:
        clientInstance();
        void display(void);

        void setVerbose(bool);
        bool getVerbose();

        int cmdSet( char *name, char *value);
        char *cmdGet( char *name, int *error);

        ~clientInstance();
};

#endif
