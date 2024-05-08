#ifndef MAILBOX_MAP_HPP
#define MAILBOX_MAP_HPP
/*********************************************************************
*
*   HEADER:
*       header file for mailbox API Mapping
*
*   Copyright 2024 Nate Lenze
*
**********************************************************************/
/*--------------------------------------------------------------------
                              INCLUDES
--------------------------------------------------------------------*/
#include "sys_def.h"
#include "messageAPI.hpp"
/*--------------------------------------------------------------------
                          GLOBAL NAMESPACES
--------------------------------------------------------------------*/
namespace core {
/*--------------------------------------------------------------------
                          LITERAL CONSTANTS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                            TYPES/ENUMS
--------------------------------------------------------------------*/
typedef union 
{
    float    flt;
    uint32_t u32;
}data_union;

typedef enum
{
    FLOAT_TYPE,
    UINT_TYPE,

    NUM_TYPES
}data_type;

typedef enum
{
    MESSAGE_API_ENGINE,

    NUM_ENGINES
}engine_type;

typedef enum
{
    NO_FLAG,
    UPDATE_TX_FLAG,
    UPDATE_RX_FLAG,

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


typedef struct
{
    data_union        data;
    data_type         type;
    update_rate       upt_rt;
    engine_type       engine; 
    flag_type         flag;
    core::location    destination;
    core::location    source;
} mabilbox_type;



/*--------------------------------------------------------------------
                           MEMORY CONSTANTS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                              VARIABLES
--------------------------------------------------------------------*/
/*----------------------------------------
global_mailbox is to be defined on each 
unit and is not included 
----------------------------------------*/
extern mabilbox_type global_mailbox[];
/*--------------------------------------------------------------------
                                MACROS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                              PROCEDURES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                               CLASSES
--------------------------------------------------------------------*/



} /* namespace core */
#endif