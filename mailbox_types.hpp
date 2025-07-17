#ifndef MAILBOX_TYPES_HPP
#define MAILBOX_TYPES_HPP
/*********************************************************************
*
*   HEADER:
*       header file for mailbox types
*
*   Copyright 2025 Nate Lenze
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

typedef union                 /* data union                         */
    {
    float        flt32;       /* float32 data                       */
    int          uint32;      /* uint32 data                        */
    bool         boolean;     /* boolean data                       */
    uint8_t      raw_data[4]; /* raw data accessor
                                 NOTE: RP2040 is a 32bit processor so 
                                 float & int are 32 bits, hence 
                                 raw_data can access bytes          */
    }data_union;

enum struct data_type /* data types for data_union                  */
    {
    FLOAT_32_TYPE,    /* flt32 data type                            */
    UINT_32_TYPE,     /* uint32 data type                           */
    BOOLEAN_TYPE,     /* boolean data type                          */

    NUM_TYPES         /* number of data types                       */
    };

enum struct flag_type /* mailbox flag (status)                      */
    {
    NO_FLAG,          /* no flag present                            */
    TRANSMIT_FLAG,    /* transmit flag present                      */
    RECEIVE_FLAG,     /* receive flag present                       */

    NUM_FLAGS         /* number of flag types                       */
    };

enum struct update_rate : int /* Update rate (in rounds)            */
{
    RT_1_ROUND  = 1,          /* Update every (1) rounds            */
    RT_5_ROUND  = 5,          /* Update every (1) rounds            */
    RT_10_ROUND = 10,         /* Update every (10) rounds           */
    RT_ASYNC    = 0xFF,       /* Update as data is updated (async)  */

    NUM_UPDATE_RATES = 4      /* number of update rates             */
};
const std::unordered_map< data_type, int> size_map /* size mapping of
                                                      data_types to 
                                                      byte size     */
    {
    { data_type::FLOAT_32_TYPE, 4 },               /* flt32 size    */
    { data_type::UINT_32_TYPE,  4 },               /* uint32 size   */
    { data_type::BOOLEAN_TYPE,  1 }                /* boolean size  */
    };
typedef struct                     /* mailbox entry format          */
    {
    data_union        data;        /* data entry                    */
    data_type         type;        /* data type                     */
    update_rate       upt_rt;      /* update rate (in rounds)       */
    flag_type         flag;        /* data flag (status)            */
    location          destination; /* data destination              */
    location          source;      /* data source                   */
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