#ifndef MAILBOX_MAP_TYPES_HPP
#define MAILBOX_MAP_TYPES_HPP
/*********************************************************************
*
*   HEADER:
*       enum definitions for mailbox API. enum mbx_index must match
*       global_mailbox in order for mailbox API to function properly.
*
*   Copyright 2025 Nate Lenze
*
*********************************************************************/

/*--------------------------------------------------------------------
                           GENERAL INCLUDES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                           MEMORY CONSTANTS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                          LITERAL CONSTANTS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                                TYPES
--------------------------------------------------------------------*/
enum struct mbx_index : uint8_t
    {
    FLOAT_TX_FROM_RPI_MSG,
    FLOAT_RX_FROM_RPI_MSG,
    INT_TX_FROM_RPI_MSG,
    INT_RX_FROM_RPI_MSG,
    BOOL_TX_FROM_RPI_MSG,
    BOOL_RX_FROM_RPI_MSG,
    ASYNC_TX_FROM_RPI_MSG,
    ASYNC_RX_FROM_RPI_MSG,
    RND_5_TX_FROM_RPI_MSG,
    RND_5_RX_FROM_RPI_MSG,
    TEST_TX_FROM_RPI_MSG,
    TEST_RX_FROM_RPI_MSG,

    NUM_MAILBOX,
    MAILBOX_NONE,
    
    RESERVED_1 = 0xFF, //ack ID
    RESERVED_2 = 0xFE //round update
    
    //this NEEDS TO BE CONVERTED TO a struct enum w/ constexpr: https://stackoverflow.com/questions/70318806/avoiding-repetitive-copy-paste-of-static-castsize-tenum-type-for-casting-an
    //causes other issues though like casting an int into type mbx_index... what does that mean? casting to a struct w/ 6 variables? size != 1 int
    };

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

/* mailbox_map_types.hpp */
#endif