#include "mailbox_types.hpp"
#include "mailbox.hpp"
#include "sys_def.h"


mailbox_type global_mailbox[] = 
{
/* data, type,        updt_rt, flag,    direction, destination, source       */
{ 0,     INT_TYPE,    AYSNC,   NO_FLAG, RX,        RPI_MODULE,  PICO_MODULE }, /* EXAMPLE_INT_MSG */
{ 0.0f,  FLOAT_TYPE,  500_MS,  NO_FLAG, TX,        PICO_MODULE, RPI_MODULE  }  /* EXAMPLE_FLT_MSG */
};


core::mailbox( global_mailbox, NUM_MSG );