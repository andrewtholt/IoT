
#ifndef __MY_ERRORS
#define __MY_ERRORS

#define OK 0
#define CLIENTEXIT 0xff // not realy an error, just convenient.

#define GENERALERROR 0x01
#define UNKNOWNDBTYPE 0x02

#define PARSER 0x10
#define MQTT 0x20
#define DATABASE 0x80

#define PARSER_OK (PARSER | OK )
#define MQTT_OK (MQTT | OK )
#define DATABASE_OK (DATABASE | OK )

// Parse errors
//
#define NOTACMD 0x01
#define NOTIMPLEMENTED 0x02

#define CONNECTFAIL 0x01
#define ALREADYCONNECTED 0x02
#define NOTCONNECTED 0x04
#define UNKNOWN 0x08

// MQTT Errors
//
#define INVALID_PARAMS 0x01
#define CHECK_ERRNO 0x02
#endif
