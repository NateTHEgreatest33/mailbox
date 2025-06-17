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
#define TESTING (true)
/*--------------------------------------------------------------------
                                TYPES
--------------------------------------------------------------------*/
enum struct mbx_index : uint8_t
    {
#ifndef TESTING
    EXAMPLE_INT_MSG,
    EXAMPLE_FLT_MSG,
    EXAMPLE_RX_MSG,
    EXAMPLE_FLT_RX_MSG,
#else
    FLOAT_TX_FROM_RPI_MSG, /* */
    FLOAT_RX_FROM_RPI_MSG, /* */
    INT_TX_FROM_RPI_MSG,   /* */
    INT_RX_FROM_RPI_MSG,   /* */
    BOOL_TX_FROM_RPI_MSG,  /* */
    BOOL_RX_FROM_RPI_MSG,  /* */
    ASYNC_TX_FROM_RPI_MSG, /* */
    ASYNC_RX_FROM_RPI_MSG, /* */
    RND_5_TX_FROM_RPI_MSG, /* */
    RND_5_RX_FROM_RPI_MSG, /* */
    TEST_TX_FROM_RPI_MSG,  /* */
    TEST_RX_FROM_RPI_MSG,  /* */
#endif

    NUM_MAILBOX,           /* Number of Mailbox */
    MAILBOX_NONE,          /* Mailbox None      */
    
    RESERVED_1 = 0xFF,     /* ACK ID            */
    RESERVED_2 = 0xFE      /* Round Update ID   */
    
    //this NEEDS TO BE CONVERTED TO a struct enum w/ constexpr: https://stackoverflow.com/questions/70318806/avoiding-repetitive-copy-paste-of-static-castsize-tenum-type-for-casting-an
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