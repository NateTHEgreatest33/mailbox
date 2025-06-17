#ifndef MAILBOX_HPP
#define MAILBOX_HPP
/*********************************************************************
*
*   HEADER:
*       header file for mailbox API
*
*   Copyright 2025 Nate Lenze
*
**********************************************************************/
/*--------------------------------------------------------------------
                              INCLUDES
--------------------------------------------------------------------*/
#include "messageAPI.hpp"
#include "mailbox_types.hpp"
#include "queue.hpp"
#include "console.hpp"
#include "mutex_lock.hpp"

#include "mailbox_map_types.hpp"

#include <unordered_map>
#include <array>


#include "pico/mutex.h"

/*--------------------------------------------------------------------
                          GLOBAL NAMESPACES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                          LITERAL CONSTANTS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                            TYPES/ENUMS
--------------------------------------------------------------------*/
enum struct msg_type { data, update, ack, num_rtn_type };
struct msgAPI_rx
	{
    msgAPI_rx( msg_type rtn, mbx_index idx, data_union data ) : r(rtn), i(idx), d(data) {}
    msgAPI_rx() {}
	msg_type r;
	mbx_index i;
	data_union d;
	}; //if index == 0xFF --> ack

struct msgAPI_tx
    {
    msgAPI_tx( msg_type m_type, mbx_index idx ) : r(m_type), i(idx) {}
    msgAPI_tx() {}
    msg_type r;
    mbx_index i;
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

/*--------------------------------------------------------------------
                               CLASSES
--------------------------------------------------------------------*/
namespace core {

template<int M>
class mailbox
    {
    public:
        mailbox( std::array<mailbox_type, M>& global_mailbox );
        void rx_runtime( void );
        void tx_runtime( void );
        void watchdog( void );
        data_union access( mbx_index global_mbx_indx, flag_type& current_flag, bool clear_flag = true );
        bool update( data_union d, int global_mbx_indx, bool user_mode = true );
        ~mailbox( void );

    private:
        std::array<mailbox_type, M>& p_mailbox_ref;
        int p_round_cntr; /* count number of rounds that have passed, resets at 100  */
        utl::queue<(M+1), msgAPI_tx> p_transmit_queue; //is this size right? im not sure since we can ACK ROUND AND TX, in theory we could get multiple of the same & need multiple acks
        utl::queue<M, mbx_index> p_ack_queue;
        std::array<bool, M> p_awaiting_ack;
        utl::queue<M, msgAPI_rx> p_rx_queue;
        volatile int p_current_round; /* current round as set by round updater */
        mutex_t p_mailbox_protection;
        bool p_watchdog_pet;

        tx_message lora_pack_engine( void ); //this should be somewhere else, engine Tx type should have its own engine
        void lora_unpack_engine( const rx_multi msg );
        void process_tx( mbx_index index );
        void process_rx_data( mbx_index index, data_union data ); //is this needed? why dont we use public update functions?? also we need to protect data w/ mutex
        void transmit_engine( void );
        mbx_index verify_index( int idx );

    };

} /* core namespace */


/*--------------------------------------------------------------------
                        TEMPLATE INITILIZATION
--------------------------------------------------------------------*/
#include "mailbox.tpp"

#endif
