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
    unit8_t      uint8_access[4];
}data_union;

typedef enum
{
    FLOAT_32_TYPE,
    UINT_32_TYPE,
    BOOLEAN_TYPE,
    

    NUM_TYPES
}data_type;

//mapping of sizes to how many u8 they would take up
cont std::unordered_map< data_type, int> size_map
    {
    { FLOAT_32_TYPE, 4 },
    { UINT_32_TYPE,  4 },
    { BOOLEAN_TYPE,  1 }
    };

std::static_assert( size_map.size() != NUM_TYPES );

// typedef enum
// {
//     MESSAGE_API_ENGINE,

//     NUM_ENGINES
// }engine_type;

typedef enum
{
    NO_FLAG,
    TRANSMIT_FLAG,
    RECEIVE_FLAG,

    NUM_FLAGS
} flag_type;


typedef enum
{
    100_MS,
    500_MS,
    1_S,
    ASYNC,

    NUM_UPDATE_RATES
} update_rate;

typedef enum 
{
    TX,
    RX

    NUM_DIRECTINS
} direction;


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