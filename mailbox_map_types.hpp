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
    FLOAT_TX_FROM_RPI_MSG, /* testing mailboxes */
    FLOAT_RX_FROM_RPI_MSG, /* testing mailboxes */
    INT_TX_FROM_RPI_MSG,   /* testing mailboxes */
    INT_RX_FROM_RPI_MSG,   /* testing mailboxes */
    BOOL_TX_FROM_RPI_MSG,  /* testing mailboxes */
    BOOL_RX_FROM_RPI_MSG,  /* testing mailboxes */
    ASYNC_TX_FROM_RPI_MSG, /* testing mailboxes */
    ASYNC_RX_FROM_RPI_MSG, /* testing mailboxes */
    RND_5_TX_FROM_RPI_MSG, /* testing mailboxes */
    RND_5_RX_FROM_RPI_MSG, /* testing mailboxes */
    TEST_TX_FROM_RPI_MSG,  /* testing mailboxes */
    TEST_RX_FROM_RPI_MSG,  /* testing mailboxes */
#endif

    NUM_MAILBOX,           /* Number of Mailbox */
    MAILBOX_NONE,          /* Mailbox None      */
    
    RESERVED_1 = 0xFF,     /* ACK ID            */
    RESERVED_2 = 0xFE      /* Round Update ID   */
    
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