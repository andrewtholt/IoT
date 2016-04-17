
#ifndef __MY_ERRORS
#define __MY_ERRORS

#define OK 0
#define CLIENTEXIT 0xff // not realy an erro, just convenient.

#define GENERALERROR 0x01
#define UNKNOWNDBTYPE 0x02

#define PARSER 0x10
#define NOTACMD 0x01
#define NOTIMPLEMENTED 0x02

#define DATABASE 0x80
#define CONNECTFAIL 0x01
#define ALREADYCONNECTED 0x02
#define NOTCONNECTED 0x04
#define UNKNOWN 0x08

#endif
