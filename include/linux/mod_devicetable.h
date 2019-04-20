#ifndef UUID_STRING_LEN
#define	UUID_STRING_LEN		36
#define UUID_SIZE 16

typedef struct {
	__u8 b[UUID_SIZE];
} uuid_t;
#endif

#include "../../dist/include/linux/mod_devicetable.h"
