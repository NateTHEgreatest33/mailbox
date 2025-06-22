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
                         STRUCTS/TYPES/ENUMS
--------------------------------------------------------------------*/
enum struct msg_type /* message type (from un/pack engine)          */
    {
    data,            /* mailbox entry message type                  */
    update,          /* round update message type                   */
    ack,             /* ack message type                            */
    num_rtn_type     /* number of message types                     */
    };
struct msgAPI_rx /* receive message data mover                      */
	{
    msgAPI_rx( msg_type rtn, mbx_index idx, data_union data ) : r(rtn), i(idx), d(data) {}
    msgAPI_rx() {} 
	msg_type r;   /* message type                                   */
	mbx_index i;  /* message mailbox ptr                            */
	data_union d; /* message data variable                          */
	};

struct msgAPI_tx /* transmit data request mover                     */
    {
    msgAPI_tx( msg_type m_type, mbx_index idx ) : r(m_type), i(idx) {}
    msgAPI_tx() {}
    msg_type r;  /* message type                                    */
    mbx_index i; /* message mailbox ptr                             */
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
        mailbox( std::array<mailbox_type, M>& global_mailbox ); /* constructor   */
        ~mailbox();                                             /* deconstructor */

        void rx_runtime( void );                                /* rx_runtime    */
        void tx_runtime( void );                                /* tx_runtime    */
        void watchdog( void );                                  /* watchdog fn   */

        data_union access( mbx_index global_mbx_indx, flag_type& current_flag, bool clear_flag = true ); /* mailbox data access */
        bool update( data_union d, int global_mbx_indx, bool user_mode = true );                         /* mailbox data update */

    private:
        std::array<mailbox_type, M>& p_mailbox_ref;    /* global mailbox map reference  */
        int p_round_cntr;                              /* count number of rounds        */

        utl::queue<(M+1), msgAPI_tx> p_transmit_queue; /* transmit queue                */
        utl::queue<M, mbx_index> p_ack_queue;          /* ack queue                     */
        std::array<bool, M> p_awaiting_ack;            /* awaiting ack list             */
        utl::queue<M, msgAPI_rx> p_rx_queue;           /* receive queue                 */
        volatile int p_current_round;                  /* current round                 */
        mutex_t p_mailbox_protection;                  /* mailbox update mutex          */
        bool p_watchdog_pet;                           /* watchdog pet variable         */

        tx_message lora_pack_engine( void );           /* pack lora messages            */
        void lora_unpack_engine( const rx_multi msg ); /* unpack lora messages          */
        void process_tx( mbx_index index );            /* process tx data               */
        void process_rx_data( mbx_index index, data_union data ); /* process rx data    */
        void transmit_engine( void );                  /* transmit engine               */
        mbx_index verify_index( int idx );             /* verify mailbox index validity */

    };

} /* core namespace */


/*--------------------------------------------------------------------
                        TEMPLATE INITILIZATION
--------------------------------------------------------------------*/
#include "mailbox.tpp"

#endif
