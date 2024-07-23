/*********************************************************************
*
*   NAME:
*       mailbox.cpp
*
*   DESCRIPTION:
*       Interface for mailbox API
*
*   Copyright 2024 Nate Lenze
*
*********************************************************************/

/*--------------------------------------------------------------------
                              INCLUDES
--------------------------------------------------------------------*/
#include "mailbox.hpp"
#include "mailbox_map.hpp"
#include "messageAPI.hpp"
#include "console.hpp"
#include "mailbox_types.hpp"


#include <functional>
#include <type_traits> //for static assert
#include <unordered_map>

#include <string.h> //memset





/*--------------------------------------------------------------------
                          GLOBAL NAMESPACES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                          LITERAL CONSTANTS
--------------------------------------------------------------------*/
#define MAX_SIZE_GLOBAL_MAILBOX (256) //this is because the identfier needs to fit within a unint8t so max size - 256
/*--------------------------------------------------------------------
                                TYPES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                           MEMORY CONSTANTS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                              EXTERNS
--------------------------------------------------------------------*/
extern core::console console;
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
core::mailbox<M>::mailbox( std::array<mailbox_type, M>& global_mailbox )
    {

	// std::static_assert( size_map.size() != NUM_TYPES );

	// std::static_assert( (N > MAX_SIZE_GLOBAL_MAILBOX), "Mailbox size cannot be greater than MAX_SIZE_GLOBAL_MAILBOX" );
	//create max msg length (max unit8 size = 256?) and static assert the size of mailbox... or template typename T
    p_mailbox_ref = global_mailbox;
    p_internal_clk = 0;
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
core::mailbox<M>::~mailbox( void ) 
	{
	}



/*********************************************************************
*
*   PROCEDURE NAME:
*       core::mailbox::mailbox_runtime()
*
*   DESCRIPTION:
*       this to be run every 100ms (fastest update rate) to handle
*       different mailbox items
*
*********************************************************************/
template <int M>
void core::mailbox<M>::mailbox_runtime( void )
{
int i;
bool process;

i       = 0;
process = false;
const std::unordered_map< update_rate, std::function<bool(int)> > process_map = { { update_rate::RT_100_MS , [](int clk ){ return true;  }                                      },
																		 		  { update_rate::RT_500_MS , [](int clk ){ return ( clk == 0 || clk == 500 ) ? true : false;  } },
																				  { update_rate::RT_1_S    , [](int clk ){ return ( clk == 0 );  }                              },
																				  { update_rate::RT_ASYNC  , [](int clk ){ return true;  }                                      }
                                                                                };


//needs to be run here
this->receive_engine();


for( i = 0; i < M; i++ )
    {
	
	//bc map is const you need to use find 
    auto itr = process_map.find( p_mailbox_ref[i].upt_rt );
    if( itr == process_map.end() )
		{
		// console.add_assert( "unable to find update rate in process map");
		continue;
		}

    process = itr->second( p_internal_clk );

	//move onto next if not time to process
	if( process == false )
		{
		continue;
		}

	switch( p_mailbox_ref[ i ].dir )
		{
		case direction::TX:
			this->process_tx( i );
			break;

		case direction::RX:
			this->process_rx( i );
			break;

		default:
			// console.add_assert( "mailbox dir incorrectly configured" );
			break;
		}

	
    }

//update clock w/ rollover 
p_internal_clk = ( p_internal_clk + 100 ) % 1000;

//run transmit engine every 1s
if( p_internal_clk == 0 )
	this->transmit_engine();

} /* mailbox:mailbox_runtime() */



/*********************************************************************
*
*   PROCEDURE NAME:
*       ore::mailbox<M>::process_tx()
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
mailbox_type& current_mailbox = p_mailbox_ref[ index ];

/*----------------------------------------------------------
if current mailbox is ASYNC, only update when flag is 
tripped
----------------------------------------------------------*/
if( current_mailbox.upt_rt == update_rate::RT_ASYNC && current_mailbox.flag == flag_type::NO_FLAG )
	return;


p_transmit_queue.push( index );

}


/*********************************************************************
*
*   PROCEDURE NAME:
*       core::mailbox<M>::process_rx()
*
*   DESCRIPTION:
*       currently no processing is done in the rx process function. 
*		however, in the future this could be used to add timeout calc
*
*********************************************************************/
template <int M>
void core::mailbox<M>::process_rx
	( 
	mbx_index index
	)
{
} /* core::mailbox<M>::process_rx */

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
	messageAPI.send_message( lora_frame );
	}

//FUTURE UPDATES:
//send packed
//wait for ack
//update msgAPI Key

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
mbx_index mailbox_index;

/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
const std::unordered_map< data_type, int > data_size_map = { { data_type::FLOAT_32_TYPE , 4 },
															 { data_type::UINT_32_TYPE  , 4 },
															 { data_type::BOOLEAN_TYPE  , 1 } };

/*----------------------------------------------------------
Init variables
----------------------------------------------------------*/
memset( &return_msg, 0, sizeof( tx_message ) );
return_msg.destination = MODULE_NONE;
message_full           = false;
current_index          = 0;
data_size              = 0;
mailbox_index          = mbx_index::MAILBOX_NONE;

/*----------------------------------------------------------
loop untill Tx queue is empty or tx_message is full
----------------------------------------------------------*/
while( p_transmit_queue.size() > 0 || message_full )
	{
	/*------------------------------------------------------
	Aquire the current mailbox
	------------------------------------------------------*/
	mailbox_index   = p_transmit_queue.front();
	mailbox_type& current_mailbox = p_mailbox_ref[ mailbox_index ];

	/*------------------------------------------------------
	Aquire data size. This must be done using *.find() due
	to the fact that the std::map is defined as const
	------------------------------------------------------*/
    auto itr = data_size_map.find( current_mailbox.type );
	if( itr == data_size_map.end() )
		{
		// console.add_assert( "map was called with invalid key");
		memset( &return_msg, 0, sizeof( tx_message ) );
		return return_msg;
		}

    data_size = itr->second;

	/*------------------------------------------------------
	Determine if we can handle another message based upon
	how full the current message is

	+1 is to include index byte
	------------------------------------------------------*/
	if( current_index + data_size + 1 > MAX_LORA_MSG_SIZE )
		{
		message_full = true;
		continue;
		}

	/*------------------------------------------------------
	Update data desination. If MODULE_NONE, than this is the
	first time we have called this function, otherwise update
	to either MODULE_ALL or keep current destination.
	------------------------------------------------------*/
	if( return_msg.destination == MODULE_NONE )
		{
		return_msg.destination = current_mailbox.destination;
		}
	else
		{
		if( return_msg.destination != current_mailbox.destination )
			{
			return_msg.destination = MODULE_ALL;
			}
		}
	

	/*------------------------------------------------------
	Format of data is : [ index byte ] [ data byte ]...

	Add in index byte and update current_index
	------------------------------------------------------*/
	return_msg.message[current_index++] = mailbox_index;

	/*------------------------------------------------------
	memcopy data using data size of mailbox index and update
	current index
	------------------------------------------------------*/
	memcpy( return_msg.message[current_index], &(current_mailbox.data), data_size );
	current_index += data_size;

	/*------------------------------------------------------
	pop front of transmit queue
	------------------------------------------------------*/
	p_transmit_queue.pop();
	}

/*----------------------------------------------------------
return message
----------------------------------------------------------*/
return return_msg;
}

template <int M>
bool core::mailbox<M>::update( data_union d, int global_mbx_indx ){ return true; }

template <int M>
void core::mailbox<M>::receive_engine( void ){}
//idea here is that we would run this and pull/update

/*
more thoughts

1) table should be global accross units, which means terms like source and destination, make sense? no dual communication
2) TX should happen every 1s to avoid having lora in TX too long
3) TX should pack using the transmit queue and transmit engine
4) does update rate make sense or should it just always be async? not sure on this one 


*/


