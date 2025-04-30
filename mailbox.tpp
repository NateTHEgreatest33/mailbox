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
#include <type_traits> //for static assert
#include <unordered_map>

#include <string.h> //memset


#define TESTING (true)
#ifdef TESTING
#include <iostream>
#endif



/*--------------------------------------------------------------------
                          GLOBAL NAMESPACES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                          LITERAL CONSTANTS
--------------------------------------------------------------------*/
#define MAX_SIZE_GLOBAL_MAILBOX (254) //this is because the identfier needs to fit within a unint8t so max size - 256 minus ack id, update id

#define MSG_ACK_ID    (0xFF)
#define MSG_UPDATE_ID (0xFE)
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
/*--------------------------------------------------------------------
mailbox()
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
// p_mailbox_ref = global_mailbox;
p_transmit_round = 0;
// std::static_assert( size_map.size() != NUM_TYPES );

// std::static_assert( (N > MAX_SIZE_GLOBAL_MAILBOX), "Mailbox size cannot be greater than MAX_SIZE_GLOBAL_MAILBOX" );
//create max msg length (max unit8 size = 256?) and static assert the size of mailbox... or template typename T

//init mutexs
mutex_init( &p_mailbox_protection );

p_round_cntr = 0;

for( int i = 0 ; i < M; i++ )
	{
	p_awaiting_ack[i] = false;
	}
}

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
}

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
	const rx_multi msg
	)
{
/*------------------------------------------------------
Local Variables
------------------------------------------------------*/
int msg_data_index;
int msg_index;

/*------------------------------------------------------
Initilize Local Variables
------------------------------------------------------*/
msg_data_index = 0;
msg_index      = 0;

/*------------------------------------------------------
Parse through all messages received
------------------------------------------------------*/         	//data is packed [id][data....][id][data....]
while( msg_index < msg.num_messages )
	{
	msg_data_index = 0;
	rx_message rx_msg = msg.messages[msg_index];                  //add verify index here

	/*------------------------------------------------------
	Parse through all packed messages within the single 
	message
	------------------------------------------------------*/   
	while( msg_data_index < rx_msg.size )
		{
		/*------------------------------------------------------
		clear & setup data union
		------------------------------------------------------*/
		data_union data;
		data.integer = 0;

		/*------------------------------------------------------
		Handle message if it is an ack
		------------------------------------------------------*/
		if( rx_msg.message[msg_data_index] == MSG_ACK_ID )
			{
			msg_data_index++; //get to data

			//data is cleared beforehand
			msgAPI_rx rx_data( msg_type::ack, this->verify_index(rx_msg.message[msg_data_index] ), data );
			p_rx_queue.push( rx_data );

			msg_data_index++; //move onto next message
			#ifdef TESTING
				std::cout << "[ACK - " << static_cast<int>(rx_data.i) << "] | ";
			#endif

			}
		/*------------------------------------------------------
		Handle message if it is a round update
		------------------------------------------------------*/
		else if( rx_msg.message[msg_data_index] == MSG_UPDATE_ID )
			{
			// trasmit_round = (trasmit_round + 1 ) % NUM_DESTINATIONS;
			msg_data_index++; //get to data

			data.integer = rx_msg.message[msg_data_index];
			msgAPI_rx rx_data( msg_type::update, static_cast<mbx_index>(MSG_UPDATE_ID), data ); //MSG_UPDATE_ID
			p_rx_queue.push( rx_data );

			#ifdef TESTING
				std::cout << "[RND - " << rx_data.d.integer << "] | ";
			#endif

			//update past update
			msg_data_index++;
			}
		/*------------------------------------------------------
		Handle message if it is actual data
		------------------------------------------------------*/
		else
			{
			/*------------------------------------------------------
			Aquire index and verify index
			------------------------------------------------------*/
			mbx_index mailbox_index = this->verify_index( rx_msg.message[msg_data_index] );

			msg_data_index++; //get to data section

			if( mailbox_index == mbx_index::MAILBOX_NONE )
				{
				//error handle & exit
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
			int data_size = itr->second;

			/*------------------------------------------------------
			memcpy data into union
			------------------------------------------------------*/
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

				#ifdef TESTING
				std::cout << "[DATA - " << static_cast<int>(rx_data.i) << "] " << std::hex << rx_data.d.raw_data[0] << " " << rx_data.d.raw_data[1] << " " << rx_data.d.raw_data[2] << " " << rx_data.d.raw_data[3] << " | ";
				#endif

				}
				
			/*------------------------------------------------------
			Update msg_data_index based upon data size
			------------------------------------------------------*/
			msg_data_index += data_size;
			}
		}

	/*------------------------------------------------------
	Move onto next message
	------------------------------------------------------*/
	msg_index++;
	}


#ifdef TESTING
	std::cout << std::endl;
#endif
}


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
rx_multi rx_data;

rx_data = messageAPI.get_multi_message();

//fast exit
if( rx_data.num_messages == 0 || rx_data.global_errors != MSG_NO_ERROR )
	return;

//unpack data
//NEED TO ADD ERROR CHECKING!! RX DATA MAY HAVE ERRORS!!!

this->lora_unpack_engine( rx_data );	

//handle p_rx_queue()
while( !p_rx_queue.is_empty() )
	{
	msgAPI_rx temp = p_rx_queue.front();

	if( p_transmit_queue.is_full() )
		{
		Console.add_assert("queue is full while we are attempting to add"); //this is wrong bc only data requires an ack
		}

	switch( temp.r )
		{
		case msg_type::data:
			{
			this->process_rx_data( temp.i, temp.d );

			//add rx'ed msg to ack list
			msgAPI_tx tx_msg( msg_type::ack, temp.i );
			p_transmit_queue.push( tx_msg );
			break;
			}

		case msg_type::ack:
			if( !p_awaiting_ack[ static_cast<uint8_t>(temp.i) ] )
				{
				//Console.add_assert( "un-requested ack received");
				}
			else
				{
				p_awaiting_ack[ static_cast<uint8_t>(temp.i) ] = false;

				//every 1s maybe do an ack verify <-- yes ack verify happens the tx_process AFTER sent
				}
			break;

		case msg_type::update:
			p_transmit_round = temp.d.integer;

			break;
		default:
			//Console.add_assert( "malformed return type" );
			break;
		}

	p_rx_queue.pop();


	}

////Console report if errors present

}

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
int i;
bool process;

i       = 0;
process = false;

//DEBUG
// p_transmit_round = current_location;


//only run when current round == current location
if( p_transmit_round != current_location )
	return;

p_watchdog_pet = true;

//loop through entire mailbox
for( i = 0; i < M; i++ )
    {
	mailbox_type& currentMbx = p_mailbox_ref[i];

	//skip any that arent tx
	if( currentMbx.dir != direction::TX )
		continue;

	if( currentMbx.upt_rt == update_rate::RT_ASYNC || ( ( p_round_cntr % static_cast<int>( currentMbx.upt_rt ) ) == 0 ) ) //issue! cant mod 0
		this->process_tx( static_cast<mbx_index>(i) );

    }

//update clock w/ rollover @ 100
p_round_cntr = ( p_round_cntr + 1) % 100;

//update transmit round
if( !p_transmit_queue.push( msgAPI_tx( msg_type::update, mbx_index::MAILBOX_NONE ) ) )
	{
	Console.add_assert("Error pushing to tx queue");
	}

//run transmit engine
this->transmit_engine();

} /* core::mailbox:mailbox_runtime() */



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
	mbx_index index	 
	)
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
mailbox_type& current_mailbox = p_mailbox_ref[ static_cast<int>(index) ];

/*----------------------------------------------------------
if current mailbox is ASYNC, only update when flag is 
tripped
----------------------------------------------------------*/
if( current_mailbox.upt_rt == update_rate::RT_ASYNC && current_mailbox.flag == flag_type::NO_FLAG )
	return;

msgAPI_tx msg_tx( msg_type::data, index );
if( !p_transmit_queue.push( msg_tx ) )
	{
	Console.add_assert("TX queue is full");
	}

}


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
Verify and clear ack queue. This is dont first in the
engine so that we can fill this in again to verify for the 
next round
----------------------------------------------------------*/
while( !p_ack_queue.is_empty() )
	{
	int current_index = static_cast<int>( p_ack_queue.front() );
	
	if( p_awaiting_ack[current_index] != false )
		{
		std::string assert_msg = "message failed to ack: " + std::to_string(current_index);
		//Console.add_assert( assert_msg );
		}

	p_ack_queue.pop();
	}

/*----------------------------------------------------------
Empty trasmit queue and tx over messageAPI
----------------------------------------------------------*/
while( !p_transmit_queue.is_empty() )
	{
	tx_message lora_frame = lora_pack_engine();   //this needs to be reworked for adding acks, but ack format is WAY different than a normal format so need to think what the right way is here, maybe a tx struct?
	if( !messageAPI.send_message( lora_frame ) )
		{
		//Console.add_assert( "MessageAPI unable to TX" ); //this is hit when it shouldnt be?
		}
	}
	// p_transmit_queue.pop();
	// }
// tx_message msg;
// msg.destination = MODULE_ALL;
// msg.size = 2;
// msg.message[0] = MSG_UPDATE_ID;
// msg.message[1] = 0x00;
// messageAPI.send_message( msg );

// p_transmit_round = 0; //disable rx

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
tx_message  return_msg;
bool        message_full;
int         current_index;
int         data_size;
mbx_index   mailbox_index;
msgAPI_tx   tx_msg;
location packet_dest;
/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
/*----------------------------------------------------------
Init variables
----------------------------------------------------------*/
memset( &return_msg, 0, sizeof( tx_message ) );
memset( &tx_msg, 0, sizeof( msgAPI_tx ) );    

return_msg.destination = MODULE_NONE;
message_full           = false;
current_index          = 0;
data_size              = 0;
mailbox_index          = mbx_index::MAILBOX_NONE;
packet_dest            = MODULE_NONE;

#ifdef TESTING
std::cout << "Sending: ";
#endif
 
/*----------------------------------------------------------
loop untill Tx queue is empty or tx_message is full
----------------------------------------------------------*/
while( p_transmit_queue.size() > 0 && !message_full )
	{
	/*------------------------------------------------------
	Aquire the current request
	------------------------------------------------------*/
	tx_msg        = p_transmit_queue.front();
	mailbox_index = tx_msg.i;

	/*------------------------------------------------------
	Initially I thought we would need this to be a reference
	to the actual mailbox, however after digging through the
	code I decided we can make a copy since we are pulling
	data and not storing it. To mitigate the risk of user
	code writing a request while we in the middle of a tran
	------------------------------------------------------*/
	mailbox_type current_mailbox;

	if( tx_msg.r != msg_type::update )
		{
		if( verify_index( static_cast<int>(mailbox_index) ) == mbx_index::MAILBOX_NONE )
			{
			memset( &return_msg, 0, sizeof( tx_message ) );
			return return_msg;
			}
		current_mailbox = p_mailbox_ref[ static_cast<int>(mailbox_index) ];
		}

	/*------------------------------------------------------
	determine data sizing based upon message type
	------------------------------------------------------*/
	switch( tx_msg.r )
		{
		case msg_type::ack:
		case msg_type::update:
			data_size = 1;
			break;

		case msg_type::data:
			{ // {} required due to creating itr inside of switch case statement, kinda weird
			/*------------------------------------------------------
			Aquire data size. This must be done using *.find() due
			to the fact that the std::map is defined as const
			------------------------------------------------------*/
			auto itr = data_size_map.find( current_mailbox.type );
			if( itr == data_size_map.end() )
				{
				// //Console.add_assert( "map was called with invalid key");                        //debug change
				memset( &return_msg, 0, sizeof( tx_message ) );
				return return_msg;
				}

			data_size = itr->second;
			break;
			}

		default:
			data_size = 0;
			memset( &return_msg, 0, sizeof( tx_message ) );
			return return_msg;
			break;
		}

	/*------------------------------------------------------
	Determine if we can handle another message based upon
	how full the current message is

	+1 is to include index byte
	------------------------------------------------------*/
	// if( current_index + data_size + 1 > MAX_LORA_MSG_SIZE ) --> MAX_LORA_MSG_SIZE is 128, does this make sense given we are packing ina messageAPI? no
	if( current_index + data_size + 1 > MAX_MSG_LENGTH )
		{
		message_full = true;
		return_msg.size = current_index;
		break;
		}

	/*------------------------------------------------------
	Determine destination based upon if item is an ack, data 
	or update 
	------------------------------------------------------*/
	switch ( tx_msg.r )
		{
		case msg_type::data:
			packet_dest = current_mailbox.destination;
			break;
		case msg_type::update:
			packet_dest = MODULE_ALL;
			break;
		case msg_type::ack:
			packet_dest = current_mailbox.source;
			break;
		default:
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
		case msg_type::ack:
			return_msg.message[current_index++] = MSG_ACK_ID;
			return_msg.message[current_index++] = static_cast<int>(mailbox_index);

			break;

		case msg_type::data:
			{
			return_msg.message[current_index++] = static_cast<int>(mailbox_index);

			/*------------------------------------------------------
			memcopy data using data size of mailbox index and update
			current index
			------------------------------------------------------*/
			// memcpy( &(return_msg.message[current_index]), &(current_mailbox.data), data_size ); 
			flag_type throwaway_flag_data;
			// data_union temp_data = this->access( mailbox_index, throwaway_flag_data );
			data_union temp_data;
			// temp_data.flt = 0.0;
			temp_data = this->access( mailbox_index, throwaway_flag_data );
			memcpy( &(return_msg.message[current_index]), &(temp_data), data_size ); 
			current_index += data_size;

			/*------------------------------------------------------
			add data to ack queue
			------------------------------------------------------*/
			p_awaiting_ack[current_index] = true;
			p_ack_queue.push( mailbox_index );

			break;
			}

		/*------------------------------------------------------
		We update our local p_transmit_round on transmit by
		doing a pre-increment within the update function. We do
		this here otherwise we would continue to transmit until
		the next module.
		------------------------------------------------------*/
		case msg_type::update:
			return_msg.message[current_index++] = MSG_UPDATE_ID;
			return_msg.message[current_index++] = ( p_transmit_round + 1) % NUM_OF_MODULES;
			p_transmit_round = ( p_transmit_round + 1) % NUM_OF_MODULES;

			break;

		default:
			break;
		}

	#ifdef TESTING

	switch( tx_msg.r )
		{
		case msg_type::ack:
			std::cout << " [ACK - " << static_cast<int>(mailbox_index) <<"] |";
			break;

		case msg_type::data:
			{
			std::cout << " [DATA - " << static_cast<int>(mailbox_index) <<"] ";

			flag_type throwaway_flag_data;
			data_union temp_data;
			temp_data = this->access( mailbox_index, throwaway_flag_data );
			std::cout << std::hex << (int)(temp_data.raw_data[0]) << " " << (int)(temp_data.raw_data[1]) << " " << (int)(temp_data.raw_data[2]) << " " << (int)(temp_data.raw_data[3]);
			std::cout <<" |";
			break;
			}

		/*------------------------------------------------------
		We update our local p_transmit_round on transmit by
		doing a pre-increment within the update function. We do
		this here otherwise we would continue to transmit until
		the next module.
		------------------------------------------------------*/
		case msg_type::update:
			std::cout << " [RND -" << p_transmit_round <<"] |";


			break;

		default:
			break;
		}
	#endif


	/*------------------------------------------------------
	pop front of transmit queue
	------------------------------------------------------*/
	p_transmit_queue.pop();
	
	}

#ifdef TESTING
	std::cout << std::endl;
#endif

/*----------------------------------------------------------
return message and set size to current index

if we dont set size, message size == 0
----------------------------------------------------------*/
return_msg.size = current_index;
return return_msg;
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
	//Console.add_assert( "Mailbox::update() is being called on data it cannot update");
	return false;
	}

/*----------------------------------------------------------
Update mailbox while protected
----------------------------------------------------------*/
	{
	// utl::mutex_lock lock( p_mailbox_protection ); //disabling for the time being

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
rtn_data.integer = 0xFFFF; //need a better way of determining if flag or not
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
	// utl::mutex_lock lock( p_mailbox_protection ); //issue for the time being, disabling for now

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
	p_transmit_round = current_location;
	}
} /* core::mailbox::watchdog() */



// template <int M>
// void core::mailbox<M>::receive_engine( void ){}
//idea here is that we would run this and pull/update

/*
more thoughts

1) table should be global accross units, which means terms like source and destination, make sense? no dual communication
2) TX should happen every 1s to avoid having lora in TX too long
3) TX should pack using the transmit queue and transmit engine
4) does update rate make sense or should it just always be async? not sure on this one 


*/