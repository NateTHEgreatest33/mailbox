#ifndef MAILBOX_MAP_HPP
#define MAILBOX_MAP_HPP
#include "mailbox_types.hpp"
// #include "mailbox.hpp"
#include "sys_def.h"


mailbox_type global_mailbox[] = 
{
/* data, type,                     updt_rt,                 flag,              direction,            destination, source       */
{ 0,     data_type::UINT_32_TYPE,  update_rate::RT_ASYNC,   flag_type::NO_FLAG, direction::RX,        RPI_MODULE,  PICO_MODULE }, /* EXAMPLE_INT_MSG */
{ 0.0f,  data_type::FLOAT_32_TYPE, update_rate::RT_5_ROUND,  flag_type::NO_FLAG, direction::TX,        PICO_MODULE, RPI_MODULE  }  /* EXAMPLE_FLT_MSG */
};

enum struct mbx_index : uint8_t
    {
    EXAMPLE_INT_MSG,
    EXAMPLE_FLT_MSG,


    NUM_MAILBOX,
    MAILBOX_NONE,
    RESERVED_1 = 0xFF; //ack ID
    RESERVED_2 = 0xFE; //round update
    

    };
//max size is uint8t

#endif