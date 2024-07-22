#ifndef MAILBOX_MAP_HPP
#define MAILBOX_MAP_HPP
#include "mailbox_types.hpp"
// #include "mailbox.hpp"
#include "sys_def.h"


mailbox_type global_mailbox[] = 
{
/* data, type,                     updt_rt,                 flag,              direction,            destination, source       */
{ 0,     data_type::UINT_32_TYPE,  update_rate::RT_ASYNC,   flag_type::NO_FLAG, direction::RX,        RPI_MODULE,  PICO_MODULE }, /* EXAMPLE_INT_MSG */
{ 0.0f,  data_type::FLOAT_32_TYPE, update_rate::RT_500_MS,  flag_type::NO_FLAG, direction::TX,        PICO_MODULE, RPI_MODULE  }  /* EXAMPLE_FLT_MSG */
};

enum struct letter_name
    {
    EXAMPLE_INT_MSG,
    EXAMPLE_FLT_MSG,


    NUM_LETTERS,
    LETTER_NONE
    };
//max length limited to uint_8t size



// core::mailbox<NUM_LETTERS>( global_mailbox);

#endif