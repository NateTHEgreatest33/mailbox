/*********************************************************************
*
*   NAME:
*       mailbox.cpp
*
*   DESCRIPTION:
*       Interface for mailbox API
*
*   Copyright 2025 Nate Lenze
*
*********************************************************************/

/*--------------------------------------------------------------------
                              INCLUDES
--------------------------------------------------------------------*/
#include "mailbox.hpp"
#include "messageAPI.hpp"
#include "Console.hpp"
#include "mailbox_types.hpp"

#include <functional>
#include <type_traits>
#include <unordered_map>
#include <string.h>

/*--------------------------------------------------------------------
                          GLOBAL NAMESPACES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                          LITERAL CONSTANTS
--------------------------------------------------------------------*/
#define MAX_SIZE_GLOBAL_MAILBOX ( 254  ) /* Max size of a global 
											mailbox: 256 minus ack id
											and update id 		   */
#define MSG_ACK_ID              ( 0xFF ) /* ACK identifier         */
#define MSG_UPDATE_ID           ( 0xFE ) /* Round Update identifier
																   */

#define RND_CNTR_ROLLOVER       ( 100  ) /* Round rollover value   */
#define INDEX_BYTE_SIZE         ( 1    ) /* size of index byte in 
											message				   */

#define TX_ERR_MASK				( 0x18 ) /* TX runtime error mask  */
#define RX_ERR_MASK				( 0x0F ) /* RX runtime error mask  */
#define ALL_ERR_MASK 			( 0xFF ) /* All error mask  	   */
/*--------------------------------------------------------------------
                                TYPES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                           MEMORY CONSTANTS
--------------------------------------------------------------------*/
const std::unordered_map< data_type, int > data_size_map = { { data_type::FLOAT_32_TYPE , 4 },
															 { data_type::UINT_32_TYPE  , 4 },
															 { data_type::BOOLEAN_TYPE  , 1 } };
															 
/*--------------------------------------------------------------------
                              EXTERNS
--------------------------------------------------------------------*/
extern core::console Console;
extern const location current_location;
extern core::messageInterface messageAPI;

/*--------------------------------------------------------------------
                              VARIABLES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                                MACROS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                              PROCEDURES
--------------------------------------------------------------------*/
/*********************************************************************
*
*   PROCEDURE NAME:
*       core::mailbox::mailbox (constructor)
*
*   DESCRIPTION:
*       mailbox class constructor
*
*********************************************************************/
template <int M>
core::mailbox<M>::mailbox
	(
	std::array<mailbox_type, M>& global_mailbox 
	) :
	p_mailbox_ref( global_mailbox )
{
/*------------------------------------------------------
Initilize current round and counter to zero. This is done 
on all units upon startup to avoid a multi-broadcast
------------------------------------------------------*/
p_current_round = 0;
p_round_cntr    = 0;

/*------------------------------------------------------
initilize mailbox access mutex 
------------------------------------------------------*/
mutex_init( &p_mailbox_protection );

/*------------------------------------------------------
initilize p_awaiting_ack array to false
------------------------------------------------------*/
memset(&p_awaiting_ack, 0, sizeof(bool)*M );

} /* core::mailbox<M>::mailbox() */

/*********************************************************************
*
*   PROCEDURE NAME:
*       core::mailbox::~mailbox (deconstructor)
*
*   DESCRIPTION:
*       mailbox class deconstructor
*
*********************************************************************/
template <int M>
core::mailbox<M>::~mailbox
	(
	void 
	) 
{
/*------------------------------------------------------
left blank intentionally
------------------------------------------------------*/
} /* core::mailbox<M>::~mailbox() */

/*********************************************************************
*
*   PROCEDURE NAME:
*       core::mailbox::lora_unpack_engine
*
*   DESCRIPTION:
*       unpack messageAPI messages
*
*********************************************************************/
template <int M>
void core::mailbox<M>::lora_unpack_engine
	(
	const rx_multi msg /* loraAPI message object      */
	)
{
/*------------------------------------------------------
Local Variables
------------------------------------------------------*/
int msg_data_index; /* data index for each lora msg   */
int msg_index;      /* msg index for each lora msg    */
rx_message rx_msg;  /* single messageAPI object       */

/*------------------------------------------------------
Initilize Local Variables
------------------------------------------------------*/
msg_data_index = 0;
msg_index      = 0;
memset( &rx_msg, 0, sizeof(rx_message) );

/*------------------------------------------------------
Parse through all messages received
------------------------------------------------------*/
for( msg_index = 0; msg_index < msg.num_messages; msg_index++ )
	{
	msg_data_index = 0;
	rx_msg = msg.messages[msg_index]; 

	/*------------------------------------------------------
	skip message if errors are present
	------------------------------------------------------*/ 
	if( msg.errors[msg_index] != MSG_NO_ERROR )
		{
		this->log_error(mailbox_error_types::RX_MSG_API_ERR);
		continue;
		}

	/*------------------------------------------------------
	Parse through all packed messages within the single 
	message. Data is packed in the following format:
	[idx][data....][ack][idx][rnd][rnd]
	------------------------------------------------------*/   
	while( msg_data_index < rx_msg.size )
		{
		/*------------------------------------------------------
		clear & setup data union
		------------------------------------------------------*/
		data_union data;
		data.uint32 = 0;

		/*------------------------------------------------------
		Handle message if it is an ack

		Format is [ACK_ID][Index]
		------------------------------------------------------*/
		if( rx_msg.message[msg_data_index] == MSG_ACK_ID )
			{
			/*--------------------------------------------------
			Update pointer to index portion
			--------------------------------------------------*/
			msg_data_index++;

			/*--------------------------------------------------
			Generate rx queue object for ack & add to queue
			--------------------------------------------------*/
			msgAPI_rx rx_data( msg_type::ack, this->verify_index(rx_msg.message[msg_data_index] ), data );
			p_rx_queue.push( rx_data );

			/*--------------------------------------------------
			Update pointer for processing
			--------------------------------------------------*/
			msg_data_index++;

			}
		/*------------------------------------------------------
		Handle message if it is a round update

		Format is [RND_ID][new_round]
		------------------------------------------------------*/
		else if( rx_msg.message[msg_data_index] == MSG_UPDATE_ID )
			{
			/*--------------------------------------------------
			Update pointer to new round portion
			--------------------------------------------------*/
			msg_data_index++;

			/*--------------------------------------------------
			Generate rx queue object for round update & add to 
			queue
			--------------------------------------------------*/
			data.uint32 = rx_msg.message[msg_data_index];
			msgAPI_rx rx_data( msg_type::update, static_cast<mbx_index>(MSG_UPDATE_ID), data );
			p_rx_queue.push( rx_data );

			/*--------------------------------------------------
			Update pointer for processing
			--------------------------------------------------*/
			msg_data_index++;
			}
		/*------------------------------------------------------
		Handle message if it is actual data

		Format is [Index][data...]
		------------------------------------------------------*/
		else
			{
			/*------------------------------------------------------
			Aquire index and verify index
			------------------------------------------------------*/
			mbx_index mailbox_index = this->verify_index( rx_msg.message[msg_data_index] );

			/*--------------------------------------------------
			Update pointer to data portion
			--------------------------------------------------*/
			msg_data_index++;

			/*--------------------------------------------------
			If index is invalid assert and break from processing
			this message and continue to next message in rx_multi
			--------------------------------------------------*/
			if( mailbox_index == mbx_index::MAILBOX_NONE )
					{
					this->log_error(mailbox_error_types::RX_INVALID_IDX);
					break;
					}

			/*------------------------------------------------------
			Aquire mailbox from index
			------------------------------------------------------*/
			mailbox_type& current_mailbox = p_mailbox_ref[ static_cast<int>(mailbox_index) ];

			/*------------------------------------------------------
			Aquire data size. This must be done using *.find() due
			to the fact that the std::map is defined as const
			------------------------------------------------------*/
			auto itr = data_size_map.find( current_mailbox.type );
			if (itr == data_size_map.end())
				{
				this->log_error(mailbox_error_types::ENGINE_FAILURE);
				break; 
				}
			int data_size = itr->second;

			/*------------------------------------------------------
			verify size & memcpy data into union
			------------------------------------------------------*/
			if (msg_data_index + data_size > rx_msg.size)
				{
				this->log_error(mailbox_error_types::RX_MSG_OVERFLOW);
				break; 
				}
			memcpy( &data, &(rx_msg.message[msg_data_index]), data_size );

			/*------------------------------------------------------
			Determine if data was meant for us or was just packaged
			on a destination ALL message. If it is meant for us, add
			to queue
			------------------------------------------------------*/
			if( current_mailbox.destination == current_location || 
				current_mailbox.destination == MODULE_ALL          )
				{
				/*--------------------------------------------------
				Add data to queue
				--------------------------------------------------*/
				msgAPI_rx rx_data( msg_type::data, mailbox_index, data );
				p_rx_queue.push( rx_data );
				}
				
			/*------------------------------------------------------
			Update msg_data_index based upon data size
			------------------------------------------------------*/
			msg_data_index += data_size;
			}
		}
	}

} /* core::mailbox::lora_unpack_engine() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       core::mailbox::rx_runtime()
*
*   DESCRIPTION:
*       this to be run significatly faster than tx_runtime. Because of
*		the design of loraAPI and messageAPI, we need to constantly be
*		checking for new messages.
*
*********************************************************************/
template <int M>
void core::mailbox<M>::rx_runtime
	(
	void
	)
{
/*------------------------------------------------------
Local data
------------------------------------------------------*/
rx_multi rx_data;  /* messageAPI return object        */
msgAPI_rx temp;    /* unpacked msgAPI object          */

/*------------------------------------------------------
Initilize local data
------------------------------------------------------*/
memset( &rx_data, 0x00, sizeof(rx_multi));
memset( &temp, 0x00, sizeof(msgAPI_rx));

/*------------------------------------------------------
Aquire all messages from last run
------------------------------------------------------*/
rx_data = messageAPI.get_multi_message();

/*------------------------------------------------------
Fast exit if errors or no new messages
------------------------------------------------------*/
if( rx_data.num_messages == 0 || rx_data.global_errors != MSG_NO_ERROR )
	return;

/*------------------------------------------------------
Unpack all lora data
------------------------------------------------------*/
lora_unpack_engine( rx_data );	

/*------------------------------------------------------
Handle Rx queue
------------------------------------------------------*/
while( !p_rx_queue.is_empty() )
	{
	/*--------------------------------------------------
	Aquire front of rx queue & switch based upon message
	type
	--------------------------------------------------*/
	temp = p_rx_queue.front();

	switch( temp.r )
		{
		/*----------------------------------------------
		CASE: data message type
		----------------------------------------------*/
		case msg_type::data:
			{
			/*------------------------------------------
			Process (update) rx data
			------------------------------------------*/	
			this->process_rx_data( temp.i, temp.d );

			/*------------------------------------------
			Since we have rx'ed a index, add ack to tx
			queue. If queue is full assert
			------------------------------------------*/	
			msgAPI_tx tx_msg( msg_type::ack, temp.i );

			if( !p_transmit_queue.push( tx_msg ) )
				{
				this->log_error(mailbox_error_types::QUEUE_FULL);
				}

			break;
			}

		/*----------------------------------------------
		CASE: ack message type
		----------------------------------------------*/
		case msg_type::ack:
			{
			/*------------------------------------------
			Reset p_awaiting_ack[] entry if ack was
			expected, otherwise assert
			------------------------------------------*/
			if( p_awaiting_ack[ static_cast<uint8_t>(temp.i) ] )
				p_awaiting_ack[ static_cast<uint8_t>(temp.i) ] = false;
			else
				this->log_error(mailbox_error_types::RX_UNEXPECTED_ACK);

			break;
			}

		/*----------------------------------------------
		CASE: round update message type
		----------------------------------------------*/
		case msg_type::update:
			p_current_round = temp.d.uint32;

			break;
		/*----------------------------------------------
		DEFAULT: defensive programing
		----------------------------------------------*/
		default:
			this->log_error(mailbox_error_types::ENGINE_FAILURE);
			break;
		}

	/*--------------------------------------------------
	Pop front of queue and continue while() loop
	--------------------------------------------------*/
	p_rx_queue.pop();
	}

/*------------------------------------------------------
Error Handling
------------------------------------------------------*/
if( p_errors != mailbox_error_types::NO_ERROR )
	{
	/*--------------------------------------------------
	Generate error string
	--------------------------------------------------*/
	std::string err_str = "Rx runtime encountered errors: " + std::to_string(p_errors);
	Console.add_assert( err_str );

	/*--------------------------------------------------
	Clear errors
	--------------------------------------------------*/
	p_errors = 0x00;
	}

} /* core::mailbox<M>::rx_runtime() */

/*********************************************************************
*
*   PROCEDURE NAME:
*       core::mailbox::tx_runtime()
*
*   DESCRIPTION:
*       this to be run every 100ms (fastest update rate) to handle
*       different mailbox items
*
*********************************************************************/
template <int M>
void core::mailbox<M>::tx_runtime
	(
	void
	)
{
/*------------------------------------------------------
Local Variables
------------------------------------------------------*/
int i;                   /* index variable            */
mbx_index current_index; /* mbx_index variable        */

/*------------------------------------------------------
Initilize local variables
------------------------------------------------------*/
i             = 0;
current_index = mbx_index::MAILBOX_NONE;

/*------------------------------------------------------
Fast exit: only run tx_runtime when p_current_round is
equal to current_location
------------------------------------------------------*/
if( p_current_round != current_location )
	return;

/*------------------------------------------------------
Mark watchdog as pet
------------------------------------------------------*/
p_watchdog_pet = true;

/*------------------------------------------------------
Loop through entire mailbox
------------------------------------------------------*/
for( i = 0; i < M; i++ )
    {
	/*--------------------------------------------------
	Aquire reference to mailbox index
	--------------------------------------------------*/
	mailbox_type& currentMbx = p_mailbox_ref[i];

	/*--------------------------------------------------
	Skip index if not a TX mailbox
	--------------------------------------------------*/
	if( currentMbx.dir != direction::TX )
		continue;

	/*--------------------------------------------------
	Process TX entry if the following conditions are
	met:
	1) rate == ASYNC
	2) p_round_cntr % rate == 0. This is simply a local
	                             counter that relates to
								 rate
	--------------------------------------------------*/
	if( currentMbx.upt_rt == update_rate::RT_ASYNC || ( ( p_round_cntr % static_cast<int>( currentMbx.upt_rt ) ) == 0 ) )
		{
		this->process_tx( static_cast<mbx_index>(i) );
		}

    }

/*------------------------------------------------------
Update round counter & handle rollover
------------------------------------------------------*/
p_round_cntr = ( p_round_cntr + 1) % RND_CNTR_ROLLOVER;

/*------------------------------------------------------
Verify and clear ack queue. This is done prior to the 
transmit engine and round update in order to
1) clear ack queue (which is reset in tx_engine)
2) resend messages w/ missing Acks
------------------------------------------------------*/
while( !p_ack_queue.is_empty() )
	{
	current_index = p_ack_queue.front();

	/*--------------------------------------------------
	If awaiting ack is not false this means we have 
	missed an ack, if this is the case, resend data
	--------------------------------------------------*/
	if( p_awaiting_ack[static_cast<int>(current_index)] != false )
		{
		if( !p_transmit_queue.push( msgAPI_tx( msg_type::data, current_index ) ) )
			this->log_error(mailbox_error_types::QUEUE_FULL);
		}

	p_ack_queue.pop();
	}

/*------------------------------------------------------
Add Update transmit round to queue. If this operation
fails due to the queue being full we will not update
our local p_current_round and our round will be re-run
w/ no ack data so there should be room in the buffer.
------------------------------------------------------*/
if( !p_transmit_queue.push( msgAPI_tx( msg_type::update, mbx_index::MAILBOX_NONE ) ) )
	{
	this->log_error(mailbox_error_types::QUEUE_FULL);
	}

/*------------------------------------------------------
Run tranmit engine to handle p_transmit_queue.
------------------------------------------------------*/
this->transmit_engine();

/*------------------------------------------------------
Error Handling
------------------------------------------------------*/
if( p_errors != mailbox_error_types::NO_ERROR )
	{
	/*--------------------------------------------------
	Generate error string
	--------------------------------------------------*/
	std::string err_str = "Tx runtime encountered errors: " + std::to_string(p_errors);
	Console.add_assert( err_str );

	/*--------------------------------------------------
	Clear errors
	--------------------------------------------------*/
	p_errors = 0x00;
	}

} /* core::mailbox<M>::tx_runtime */

/*********************************************************************
*
*   PROCEDURE NAME:
*       core::mailbox<M>::process_tx()
*
*   DESCRIPTION:
*       process a tx entry in the global mailbox table
*
*********************************************************************/
template <int M>
void core::mailbox<M>::process_tx
	( 
	mbx_index index	/* mailbox index to process */
	)
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
mailbox_type& current_mailbox = p_mailbox_ref[ static_cast<int>(index) ];
msgAPI_tx msg_tx( msg_type::data, index );

/*----------------------------------------------------------
If current mailbox is ASYNC we only update when flag is 
tripped. Exit if no flag
----------------------------------------------------------*/
if( current_mailbox.upt_rt == update_rate::RT_ASYNC && current_mailbox.flag == flag_type::NO_FLAG )
	return;

/*----------------------------------------------------------
Add msgAPI_tx object to Tx queue
----------------------------------------------------------*/
if( !p_transmit_queue.push( msg_tx ) )
	this->log_error(mailbox_error_types::QUEUE_FULL);

} /* core::mailbox<M>::process_tx() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       core::mailbox<M>::process_rx_data()
*
*   DESCRIPTION:
*       currently no processing is done in the rx process function. 
*		however, in the future this could be used to add timeout calc
*
*********************************************************************/
template <int M>
void core::mailbox<M>::process_rx_data
	( 
	mbx_index index,
	data_union data
	)
{

//TODO: come back to this function & decide if we even want it


/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
// mailbox_type& current_mailbox = p_mailbox_ref[ static_cast<int>(index) ];

/*----------------------------------------------------------
Update data, flag as not user_mode
----------------------------------------------------------*/
// current_mailbox.data = data;
this->update( data, static_cast<int>(index), false );

/*----------------------------------------------------------
if ASYNC update flag
----------------------------------------------------------*/
// if( current_mailbox.upt_rt == update_rate::RT_ASYNC )
// 	current_mailbox.flag = flag_type::RECEIVE_FLAG;

} /* core::mailbox<M>::process_rx_data */

/*********************************************************************
*
*   PROCEDURE NAME:
*       core::mailbox<M>::transmit_engine()
*
*   DESCRIPTION:
*       this function clears the transmit queue by packing messages 
*       and transmitting them using the messageAPI. 
*
*   NOTE:
*		we *NEED* to add ack support within here as currently lora
*       is setup to allways be rx'ing and tx once in awhile
*
*********************************************************************/
template <int M>
void core::mailbox<M>::transmit_engine
	(
	void
	)
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
/*----------------------------------------------------------
Init variables
----------------------------------------------------------*/

/*----------------------------------------------------------
Empty trasmit queue and tx over messageAPI
----------------------------------------------------------*/
while( !p_transmit_queue.is_empty() )
	{
	tx_message lora_frame = lora_pack_engine();  

	if( !messageAPI.send_message( lora_frame ) )
		{
		this->log_error(mailbox_error_types::TX_MSG_API_ERR);
		}
	}

} /* core::mailbox<M>::transmit_engine */

/*********************************************************************
*
*   PROCEDURE NAME:
*       core::mailbox::pack_engine()
*
*   DESCRIPTION:
*       this takes the current p_transmit_queue and packs it into
*       tx_messages's. This will run until the message is full and return
*       that message
*
*
*   NOTE:
*       this may need to be run multiple times if there are many items
*		in the transmit queue
*
*********************************************************************/
template <int M>
tx_message core::mailbox<M>::lora_pack_engine
	(
	void
	)
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
tx_message  return_msg;        /* */
bool        message_full;      /* */
int         current_index;     /* */
int         data_size;         /* */
mbx_index   mailbox_index;     /* */
msgAPI_tx   tx_msg;            /* */
location packet_dest;          /* */
mailbox_type current_mailbox;  /* */
flag_type throwaway_flag_data; /* */
data_union temp_data;          /* */
/*----------------------------------------------------------
Init variables
----------------------------------------------------------*/
memset( &return_msg, 0, sizeof( tx_message ) );
memset( &tx_msg, 0, sizeof( msgAPI_tx ) );   
memset( &current_mailbox, 0, sizeof(mailbox_type)); 

return_msg.destination = MODULE_NONE;
message_full           = false;
current_index          = 0;
data_size              = 0;
mailbox_index          = mbx_index::MAILBOX_NONE;
packet_dest            = MODULE_NONE;
 
/*----------------------------------------------------------
loop untill Tx queue is empty or tx_message is full
----------------------------------------------------------*/
while( p_transmit_queue.size() > 0 && !message_full )
	{
	/*------------------------------------------------------
	Aquire the current request & reset loop data
	------------------------------------------------------*/
	tx_msg        = p_transmit_queue.front();
	mailbox_index = tx_msg.i;

	memset( &current_mailbox, 0, sizeof(mailbox_type));

	/*------------------------------------------------------
	Verify index, assuming we are not an msg::type_update (
	which has no index)
	------------------------------------------------------*/
	if( tx_msg.r != msg_type::update )
		{
		if( verify_index( static_cast<int>(mailbox_index) ) == mbx_index::MAILBOX_NONE )
			{
			/*----------------------------------------------
			clear out return data and exit
			----------------------------------------------*/
			memset( &return_msg, 0, sizeof( tx_message ) );
			this->log_error(mailbox_error_types::RX_INVALID_IDX);
			return return_msg;
			}

		current_mailbox = p_mailbox_ref[ static_cast<int>(mailbox_index) ];
		}

	/*------------------------------------------------------
	determine data sizing based upon message type
	------------------------------------------------------*/
	switch( tx_msg.r )
		{
		/*--------------------------------------------------
		CASE: msg_type::ack
		CASE: msg_type::update

		intentional fallthrough as ack/update data size are
		the same
		--------------------------------------------------*/
		case msg_type::ack:
		case msg_type::update:
			data_size = 1;
			break;

		/*--------------------------------------------------
		CASE: msg_type::data
		--------------------------------------------------*/
		case msg_type::data:
			{
			/*----------------------------------------------
			Aquire data size. This must be done using 
			*.find() due to the fact that the std::map is 
			defined as const
			----------------------------------------------*/
			auto itr = data_size_map.find( current_mailbox.type );
			if( itr == data_size_map.end() )
				{
				/*------------------------------------------
				if no size was found in data_size_map exit 
				null msg. this should not happen but is 
				defensive programing
				------------------------------------------*/
				memset( &return_msg, 0, sizeof( tx_message ) );
				return return_msg;
				}
			/*----------------------------------------------
			Set data size
			----------------------------------------------*/
			data_size = itr->second;
			break;
			}

		/*--------------------------------------------------
		CASE: default case (defensive programing)

		Exit since unexpected case is reached
		--------------------------------------------------*/
		default:
			this->log_error(mailbox_error_types::ENGINE_FAILURE);
			data_size = 0;
			memset( &return_msg, 0, sizeof( tx_message ) );
			return return_msg;
			break;
		}

	/*------------------------------------------------------
	Determine if we can handle another message based upon
	how full the current message is. If we are full, break
	from loop

	+1 (INDEX_BYTE_SIZE) is to include index byte in
	addition to data
	------------------------------------------------------*/
	if( current_index + data_size + INDEX_BYTE_SIZE > MAX_MSG_LENGTH )
		{
		message_full    = true;
		return_msg.size = current_index;
		break;
		}

	/*------------------------------------------------------
	Determine destination based upon if item is an ack, data 
	or update 
	------------------------------------------------------*/
	switch ( tx_msg.r )
		{
		/*--------------------------------------------------
		CASE: msg_type::data
		--------------------------------------------------*/
		case msg_type::data:
			packet_dest = current_mailbox.destination;
			break;
		
		/*--------------------------------------------------
		CASE: msg_type::update
		--------------------------------------------------*/
		case msg_type::update:
			packet_dest = MODULE_ALL;
			break;
		
		/*--------------------------------------------------
		CASE: msg_type::ack
		--------------------------------------------------*/
		case msg_type::ack:
			packet_dest = current_mailbox.source;
			break;
		
		/*--------------------------------------------------
		CASE: default case (defense programing). This should
		not be able to be hit
		--------------------------------------------------*/
		default:
			this->log_error(mailbox_error_types::ENGINE_FAILURE);
			break;
		}

	/*------------------------------------------------------
	Update data desination. If MODULE_NONE, than this is the
	first time we have called this function, otherwise update
	to either MODULE_ALL or keep current destination.
	------------------------------------------------------*/
	if( return_msg.destination == MODULE_NONE )
		{
		return_msg.destination = packet_dest;
		}
	else
		{
		if( return_msg.destination != packet_dest )
			{
			return_msg.destination = MODULE_ALL;
			}
		}
	

	/*------------------------------------------------------
	Format of data is   : [ index byte   ] [ data byte ]...
	Format of ack is    : [ ack byte     ] [ index     ]
	Format of update is : [ update byete ] [new round  ]

	Add in index byte and update current_index
	------------------------------------------------------*/
	switch( tx_msg.r )
		{
		/*--------------------------------------------------
		CASE: msg_type::ack
		--------------------------------------------------*/
		case msg_type::ack:
			return_msg.message[current_index++] = MSG_ACK_ID;
			return_msg.message[current_index++] = static_cast<int>(mailbox_index);
			break;

		/*--------------------------------------------------
		CASE: msg_type::data
		--------------------------------------------------*/
		case msg_type::data:
			return_msg.message[current_index++] = static_cast<int>(mailbox_index);

			/*----------------------------------------------
			Clear flag and temp_data variables
			----------------------------------------------*/
			memset( &throwaway_flag_data, 0, sizeof(flag_type) );
			memset( &temp_data, 0, sizeof(data_union) );

			/*----------------------------------------------
			Access (mutex protected) and memcopy data 
			----------------------------------------------*/
			temp_data = this->access( mailbox_index, throwaway_flag_data );
			memcpy( &(return_msg.message[current_index]), &(temp_data), data_size ); 

			/*----------------------------------------------
			Update index based upon data size
			----------------------------------------------*/
			current_index += data_size;

			/*----------------------------------------------
			Add data to ack queue
			----------------------------------------------*/
			p_awaiting_ack[static_cast<int>(mailbox_index)] = true;

			if( !p_ack_queue.push( mailbox_index ) )
				p_errors |= mailbox_error_types::QUEUE_FULL;

			break;

		/*--------------------------------------------------
		CASE: msg_type::update
		
		We update our local p_transmit_round here to keep
		the current (this) module in sync as it will not
		Rx this message due to Lora being in TX mode
		--------------------------------------------------*/
		case msg_type::update:
			return_msg.message[current_index++] = MSG_UPDATE_ID;
			return_msg.message[current_index++] = update_round();
			break;

		/*--------------------------------------------------
		CASE: default case (defensive programing). This
		should not be possible to hit
		--------------------------------------------------*/
		default:
			this->log_error(mailbox_error_types::ENGINE_FAILURE);
			break;
		}

	/*------------------------------------------------------
	pop front of transmit queue
	------------------------------------------------------*/
	p_transmit_queue.pop();
	
	}

/*----------------------------------------------------------
return message and set size to current index
----------------------------------------------------------*/
return_msg.size = current_index;
return return_msg;
}

/*********************************************************************
*
*   PROCEDURE NAME:
*       core::mailbox<M>::log_error()
*
*   DESCRIPTION:
*       This is a private function that logs an error
*
*   NOTE:
*
*********************************************************************/
template <int M>
void core::mailbox<M>::log_error( mailbox_error_types err )
{
	p_errors |= err;
}



/*********************************************************************
*
*   PROCEDURE NAME:
*       core::mailbox::update()
*
*   DESCRIPTION:
*       This is a public function that updates a mailbox data. returns
*		false if issue updating data
*
*   NOTE:
*
*********************************************************************/
template <int M>
bool core::mailbox<M>::update
	(
	data_union d,                  /* data                          */
	int global_mbx_indx,           /* mailbox index                 */
	bool user_mode                 /* application calling (default) */
	)
{ 
/*----------------------------------------------------------
Verify passed in index
----------------------------------------------------------*/
if( this->verify_index( global_mbx_indx) == mbx_index::MAILBOX_NONE )
	{
	return false;
	}
/*----------------------------------------------------------
Verify tx/rx mode is accessed correctly. The user should only
be able to set TX items, while the updater functions should
only be able to set RX items
----------------------------------------------------------*/
	if( ( user_mode && p_mailbox_ref[global_mbx_indx].dir == direction::RX   ) ||
    ( !user_mode && p_mailbox_ref[global_mbx_indx].dir == direction::TX  ) )
	{
	this->log_error(mailbox_error_types::INVALID_API_CALL);
	return false;
	}

/*----------------------------------------------------------
Update mailbox while protected
----------------------------------------------------------*/
	{
	utl::mutex_lock lock( p_mailbox_protection );

	p_mailbox_ref[global_mbx_indx].data = d;

	/*------------------------------------------------------
	set flag based upon caller
	------------------------------------------------------*/
	if( user_mode )
		{
		p_mailbox_ref[global_mbx_indx].flag = flag_type::TRANSMIT_FLAG;
		}
	else
		{
		p_mailbox_ref[global_mbx_indx].flag = flag_type::RECEIVE_FLAG;
		}

	} /* release mutex */

/*----------------------------------------------------------
return true with data having been 
----------------------------------------------------------*/
return true;

} /* core::mailbox<M>::update() */

/*********************************************************************
*
*   PROCEDURE NAME:
*       core::mailbox::access()
*
*   DESCRIPTION:
*       This is a public function that updates a mailbox data
*
*   NOTE:
*
*********************************************************************/
template <int M>
data_union core::mailbox<M>::access
	(
	mbx_index global_mbx_indx, //suggest changing this to just mbx_index??
	flag_type& current_flag,   /* returns current flag data */
	bool      clear_flag       /* default yes */
	)
{ 
/*----------------------------------------------------------
Variables
----------------------------------------------------------*/
data_union rtn_data;

/*----------------------------------------------------------
Initize Variables
----------------------------------------------------------*/
rtn_data.uint32 = 0xFFFF; //need a better way of determining if flag or not
current_flag     = flag_type::NO_FLAG;

/*----------------------------------------------------------
Verify passed in index
----------------------------------------------------------*/
if( this->verify_index( static_cast<int>(global_mbx_indx) ) == mbx_index::MAILBOX_NONE )
	{
	return rtn_data;
	}

/*----------------------------------------------------------
Aquire reference to data
----------------------------------------------------------*/
mailbox_type& current_mbx = p_mailbox_ref[ static_cast<int>(global_mbx_indx) ];

/*----------------------------------------------------------
Handle and aquire flag data while protected
----------------------------------------------------------*/
	{
	utl::mutex_lock lock( p_mailbox_protection );

	current_flag = current_mbx.flag;

	if( clear_flag )
		current_mbx.flag = flag_type::NO_FLAG;

	/*------------------------------------------------------
	return data
	------------------------------------------------------*/
	return current_mbx.data;
	}

} /* core::mailbox::access() */

/*********************************************************************
*
*   PROCEDURE NAME:
*       core::mailbox::verify_index()
*
*   DESCRIPTION:
*       Verifies an index exists within the global table
*
*   NOTE:
*
*********************************************************************/
template <int M>
mbx_index core::mailbox<M>::verify_index
	(
	int idx
	)
{
/*----------------------------------------------------------
Return mbx_index::MAILBOX_NONE if index out of bounds
----------------------------------------------------------*/
if( idx >= static_cast<int>( mbx_index::MAILBOX_NONE ) && idx >= 0 )
	return mbx_index::MAILBOX_NONE;

/*----------------------------------------------------------
return index as mbx_index type
----------------------------------------------------------*/
return static_cast<mbx_index>(idx);

} /* core::mailbox::verify_index() */

/*********************************************************************
*
*   PROCEDURE NAME:
*       core::mailbox::watchdog()
*
*   DESCRIPTION:
*       watchdog pet function that forces a Tx round if we havent
*		pet the watchdog
*
*   NOTE:
*
*********************************************************************/
template <int M>
void core::mailbox<M>::watchdog
	( 
	void 
	)
{

/*----------------------------------------------------------
If watchdog has not been set, force a transmit round
----------------------------------------------------------*/
if( !p_watchdog_pet )
	{
	p_current_round = current_location;
	}
} /* core::mailbox::watchdog() */

/*********************************************************************
*
*   PROCEDURE NAME:
*       core::mailbox::update_round()
*
*   DESCRIPTION:
*       standardized way of updating round correctly. returns new
*		round value
*
*   NOTE:
*
*********************************************************************/
template <int M>
uint8_t core::mailbox<M>::update_round
	( 
	void 
	)
{
/*----------------------------------------------------------
Update p_current_round & rollover if end of module list
----------------------------------------------------------*/
p_current_round = ( p_current_round + 1) % NUM_OF_MODULES;
return p_current_round;
} /* core::mailbox::update_round() */


/*********************************************************************
*
*   PROCEDURE NAME:
*       core::mailbox_accessor<M>::mailbox_accessor
*
*   DESCRIPTION:
*       constructor for the accessor proxy class
*
*   NOTE:
*
*********************************************************************/
template<int M>
core::mailbox_accessor<M>::mailbox_accessor(core::mailbox<M>& mbx, mbx_index idx)
    : p_mailbox(mbx), p_index(idx)
{
    // Left blank intentionally
} /* core::mailbox_accessor<M>::mailbox_accessor */

/*********************************************************************
*
*   PROCEDURE NAME:
*       core::mailbox_accessor<M>::operator=
*
*   DESCRIPTION:
*       assignment operator overload for the proxy class
*
*   NOTE:
*
*********************************************************************/
template<int M>
core::mailbox_accessor<M>& core::mailbox_accessor<M>::operator=(const data_union& data)
{
    p_mailbox.update(data, static_cast<int>(p_index), true);
    return *this;
} /* core::mailbox_accessor<M>::operator= */

/*********************************************************************
*
*   PROCEDURE NAME:
*       core::mailbox_accessor<M>::operator data_union
*
*   DESCRIPTION:
*       type conversion operator for the proxy class
*
*   NOTE:
*
*********************************************************************/
template<int M>
core::mailbox_accessor<M>::operator data_union() const
{
    flag_type dummy_flag;
    return p_mailbox.access(p_index, dummy_flag, true);
} /* core::mailbox_accessor<M>::operator data_union() */

/*********************************************************************
*
*   PROCEDURE NAME:
*       core::mailbox_accessor<M>::access_with_flag
*
*   DESCRIPTION:
*       allows access to the underlying flag
*
*   NOTE:
*
*********************************************************************/
template<int M>
data_union core::mailbox_accessor<M>::access_with_flag(flag_type& current_flag, bool clear_flag)
{
    return p_mailbox.access(p_index, current_flag, clear_flag);
} /* core::mailbox_accessor<M>::access_with_flag() */

/*********************************************************************
*
*   PROCEDURE NAME:
*       core::mailbox<M>::operator[]
*
*   DESCRIPTION:
*       [] operator overload, returns a proxy object
*
*   NOTE:
*
*********************************************************************/
template<int M>
core::mailbox_accessor<M> core::mailbox<M>::operator[](mbx_index index)
{
    return mailbox_accessor<M>(*this, index);
} /* core::mailbox<M>::operator[] */


/*
more thoughts
1) table should be global accross units, which means terms like source and destination, make sense? no dual communication -- YES! v1.1 update 
3) add back engine var to mailbox map, each "engine" will handle its on tx/rx queue, remove lora pack/unpack junk.
   interface format will have common *._add_tx_queue(), *._rx_runtime(&global_rx_queue), *.tx_runtime()?  		 		  -- Yes v1.1 update
4) change queues to std::maps OR std::array. this will allow us to mark data as "to send" and avoid duplicate acks etc.   -- Yes v1.1 update
5) update access/update using mailbox_idx + change to standard enums to avoid multiple casting							  -- Yes v1.1 update
*/