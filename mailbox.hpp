#ifndef MAILBOX_HPP
#define MAILBOX_HPP
/*********************************************************************
*
*   HEADER:
*       header file for mailbox API
*
*   Copyright 2024 Nate Lenze
*
**********************************************************************/
/*--------------------------------------------------------------------
                              INCLUDES
--------------------------------------------------------------------*/
// #include "message_map.hpp"
#include "messageAPI.hpp"
#include "mailbox_types.hpp"
#include "queue.hpp"

#include "mailbox_map.hpp"

#include <unordered_map>
#include <array>

/*--------------------------------------------------------------------
                          GLOBAL NAMESPACES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                          LITERAL CONSTANTS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                            TYPES/ENUMS
--------------------------------------------------------------------*/

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
namespace core {

template<int M>
class mailbox
    {
    public:
        mailbox( std::array<mailbox_type, M>& global_mailbox );
        void mailbox_runtime( void );
        data_union access( int global_mbx_indx );
        bool update( data_union d, int global_mbx_indx );
        ~mailbox( void );

    private:
        std::array<mailbox_type, M>& p_mailbox_ref;
        int p_internal_clk;
        utl::queue<M, mbx_index> p_transmit_queue;


        tx_message lora_pack_engine( void ); //this should be somewhere else, engine Tx type should have its own engine
        void unpack_engine(rx_message);
        void process_tx( mailbox_type& letter );
        void process_rx( mailbox_type& letter );
        void transmit_engine( void );



    };

} /* core namespace */

#endif
