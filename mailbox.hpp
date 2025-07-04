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

enum mailbox_error_types               /* error bit array defines   */
    {
    NO_ERROR          = ( 0         ), /* no errors present         */

    RX_MSG_API_ERR    = ( 0x01 << 0 ), /* message API failures in RX
                                          runtime                   */
    RX_INVALID_IDX    = ( 0x01 << 1 ), /* invalid index in RX 
                                          runtime                   */
    RX_UNEXPECTED_ACK = ( 0x01 << 2 ), /* unexpected ack in RX
                                          runtime                   */
    QUEUE_FULL        = ( 0x01 << 3 ), /* queue full when  attempting 
                                          to push to                */
    TX_MSG_API_ERR    = ( 0x01 << 4 ), /* message API failures in TX
                                          runtime                   */
    INVALID_API_CALL  = ( 0x01 << 5 ), /* invalid use of Mailbox    */
    ENGINE_FAILURE    = ( 0x01 << 6 ), /* unexpected behavior in 
                                          engine                    */
    RX_MSG_OVERFLOW   = ( 0x01 << 7 ), /* message overflow in RX
                                          runtime                   */
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
                             PROXY CLASSES
--------------------------------------------------------------------*/
namespace core {
/*--------------------------------------------------
forward declaration of mailbox class
--------------------------------------------------*/
template<int M>
class mailbox;

/*--------------------------------------------------
Proxy class to enable operator[] read/write syntax
--------------------------------------------------*/
template<int M>
class mailbox_accessor
    {
    public:
        mailbox_accessor(mailbox<M>& mbx, mbx_index idx);                                  /* constructor                  */
        mailbox_accessor& operator=(const data_union& data);                               /* assignment operator          */
        operator data_union() const;                                                       /* type conversion operator     */
        data_union access_with_flag(flag_type& current_flag, bool clear_flag = true);      /* access with flag             */

    private:
        mailbox<M>& p_mailbox;      /* reference to parent mailbox */
        const mbx_index p_index;    /* index for this accessor     */
    };


/*--------------------------------------------------------------------
                            MAILBOX CLASSE
--------------------------------------------------------------------*/
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
        
        mailbox_accessor<M> operator[](mbx_index index);      /* overload [] 
                                                                  operator      */

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
        uint8_t p_errors;                              /* error bit array               */

        tx_message lora_pack_engine( void );           /* pack lora messages            */
        void lora_unpack_engine( const rx_multi msg ); /* unpack lora messages          */
        void process_tx( mbx_index index );            /* process tx data               */
        void process_rx_data( mbx_index index, data_union data ); /* process rx data    */
        void transmit_engine( void );                  /* transmit engine               */
        void log_error( mailbox_error_types err );     /* log error                     */
        mbx_index verify_index( int idx );             /* verify mailbox index validity */
        uint8_t update_round( void );                  /* update round                  */

    };

} /* core namespace */


/*--------------------------------------------------------------------
                        TEMPLATE INITILIZATION
--------------------------------------------------------------------*/
#include "mailbox.tpp"

#endif
