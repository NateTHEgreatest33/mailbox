#ifndef MAILBOX_TYPES_HPP
#define MAILBOX_TYPES_HPP
/*********************************************************************
*
*   HEADER:
*       header file for mailbox types
*
*   Copyright 2024 Nate Lenze
*
**********************************************************************/
/*--------------------------------------------------------------------
                              INCLUDES
--------------------------------------------------------------------*/
#include "sys_def.h"

#include "messageAPI.hpp"

#include <unordered_map>
#include <stdint.h>

/*--------------------------------------------------------------------
                          GLOBAL NAMESPACES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                          LITERAL CONSTANTS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                            TYPES/ENUMS
--------------------------------------------------------------------*/
typedef union 
{
    float        flt;
    int          integer;
    bool         boolean;
    uint8_t      raw_data[4];
}data_union;

enum struct data_type
{
    FLOAT_32_TYPE,
    UINT_32_TYPE,
    BOOLEAN_TYPE,

    NUM_TYPES
};

enum struct flag_type
{
    NO_FLAG,
    TRANSMIT_FLAG,
    RECEIVE_FLAG,

    NUM_FLAGS
};


enum struct update_rate
{
    RT_100_MS,
    RT_500_MS,
    RT_1_S,
    RT_ASYNC,

    NUM_UPDATE_RATES
};

enum struct direction 
{
    TX,
    RX,

    NUM_DIRECTINS
};

//mapping of sizes to how many u8 they would take up
const std::unordered_map< data_type, int> size_map
    {
    { data_type::FLOAT_32_TYPE, 4 },
    { data_type::UINT_32_TYPE,  4 },
    { data_type::BOOLEAN_TYPE,  1 }
    };



// typedef enum
// {
//     MESSAGE_API_ENGINE,

//     NUM_ENGINES
// }engine_type;




typedef struct
{
    data_union        data;
    data_type         type;
    update_rate       upt_rt;
    // engine_type       engine; 
    flag_type         flag;
    direction         dir;
    location          destination;
    location          source;
} mailbox_type;



/*--------------------------------------------------------------------
                           MEMORY CONSTANTS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                              VARIABLES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                                MACROS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                              PROCEDURES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                               CLASSES
--------------------------------------------------------------------*/

#endif