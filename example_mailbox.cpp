#include "mailbox_types.hpp"
#include "mailbox.hpp"

typedef enum {
EXAMPLE_INT_MSG = 0,
EXAMPLE_FLT_MSG, 

NUM_MSG
}msg_type;

mailbox_type global_mailbox[] = 
{
/* data, type,    updt_rt, engine              flag,    destination, source       */
{ 0,     integer, AYSNC,   MESSAGE_API_ENGINE, NO_FLAG, RPI_MODULE,  PICO_MODULE }, /* EXAMPLE_INT_MSG */
{ 0.0f,  flt,     500_MS,  MESSAGE_API_ENGINE, NO_FLAG, PICO_MODULE, RPI_MODULE  }  /* EXAMPLE_FLT_MSG */
};


core::mailbox( global_mailbox, NUM_MSG );