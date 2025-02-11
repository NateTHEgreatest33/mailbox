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
    EXAMPLE_INT_MSG = 0,
    EXAMPLE_FLT_MSG = 1,
    EXAMPLE_RX_MSG  = 2,
    EXAMPLE_FLT_RX_MSG = 3,

    NUM_MAILBOX,
    MAILBOX_NONE,
    
    RESERVED_1 = 0xFF, //ack ID
    RESERVED_2 = 0xFE //round update
    
    //this NEEDS TO BE CONVERTED TO a struct enum w/ constexpr: https://stackoverflow.com/questions/70318806/avoiding-repetitive-copy-paste-of-static-castsize-tenum-type-for-casting-an
    //causes other issues though like casting an int into type mbx_index... what does that mean? casting to a struct w/ 6 variables? size != 1 int
    };

// struct mbx_index 
//     {
//     static constexpr uint8_t EXAMPLE_INT_MSG = 0;
//     static constexpr uint8_t EXAMPLE_FLT_MSG = 1;


//     static constexpr uint8_t NUM_MAILBOX = 2;
//     static constexpr uint8_t MAILBOX_NONE = ( mbx_index::NUM_MAILBOX + 1 );
    
//     static constexpr uint8_t RESERVED_1 = 0xFF; //ack ID
//     static constexpr uint8_t RESERVED_2 = 0xFE; //round update
//     };

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