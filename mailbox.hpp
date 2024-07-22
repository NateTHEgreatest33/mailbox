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
class mailbox  //suggestion to use templates to pass in mailbox_size and thus be able to use arrays
    {
    public:
        mailbox( mailbox_type& global_mailbox );
        void mailbox_runtime( void );
        data_union mailbox_access( int global_mbx_indx );
        ~mailbox( void );

    private:
        mailbox_type& p_mailbox_ref;
        int p_internal_clk;
        // std::queue<mailbox_type&> p_transmit_queue; //should this become std::array instead? OR a priority queue, either one
        utl::queue<M, letter_name> p_transmit_queue;
        // std::unordered_map<int, data_union> p_rx_map; //feel like we need ot have a way of timeout? maybe a queue makes sense?


        tx_message lora_pack_engine( void ); //this should be somewhere else, engine Tx type should have its own engine
        void unpack_engine(rx_message);
        void process_tx( mailbox_type& letter );
        void process_rx( mailbox_type& letter );
        void transmit_engine( void );



    };

} /* core namespace */

#endif
