#include "mailbox_types.hpp"
#include "mailbox.hpp"
#include "sys_def.h"


mailbox_type global_mailbox[] = 
{
/* data, type,        updt_rt, flag,    direction, destination, source       */
{ 0,     INT_TYPE,    AYSNC,   NO_FLAG, RX,        RPI_MODULE,  PICO_MODULE }, /* EXAMPLE_INT_MSG */
{ 0.0f,  FLOAT_TYPE,  500_MS,  NO_FLAG, TX,        PICO_MODULE, RPI_MODULE  }  /* EXAMPLE_FLT_MSG */
};

typedef enum letter_name
    {
    EXAMPLE_INT_MSG,
    EXAMPLE_FLT_MSG,


    NUM_LETTERS,
    LETTER_NONE
    };
//max length limited to uint_8t size



core::mailbox<NUM_LETTERS>( global_mailbox);