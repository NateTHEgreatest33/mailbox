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

#include <functional>
#include <type_traits> //for static assert



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
core::mailbox<M>::mailbox( mailbox_type& global_mailbox )
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
const std::unordered_map< update_rate, std::function<bool(int)> > process_map = { { update_rate::RT_100_MS : [](int clk ){ return true;  }                                      },
																		 		  { update_rate::RT_500_MS : [](int clk ){ return ( clk == 0 || clk == 500 ) ? true : false;  } },
																				  { update_rate::RT_1_S    : [](int clk ){ return ( clk == 0 );  }                              },
																				  { update_rate::RT_ASYNC  : [](int clk ){ return true;  }                                      }
                                                                                };
i       = 0;
process = false;

// p_transmit_queue.empty();

for( i = 0; i < p_mailbox_size; i++ )
    {

	process = process_map[ p_mailbox_ref[ i ].upt_rt ]( p_internal_clk);

	//move onto next if not time to process
	if( process == false )
		{
		continue;
		}

	switch( p_mailbox_ref[ i ].dir )
		{
		case TX:
			this->process_tx( p_mailbox_ref[ i ] );
			break;
		case RX:
			this->process_rx( p_mailbox_ref[ i ] );
			break;
		default:
			console.add_assert( "mailbox dir incorrectly configured" );
			break;
		}

	
    }

//update clock
p_internal_clk = ( p_internal_clk + 100 ) % 1000;

//run transmit engine every 1s
if( p_internal_clk == 0 )
	this->transmit_engine();

} /* mailbox:mailbox_runtime() */

template <int M>
void core::mailbox<M>::process_tx
	( 
	mailbox_type& letter 
	)
{
if( source != current_unit )
	return;

//add to transmit queue if needing to be transmitted. thinking 
}

template <int M>
void core::mailbox<M>::process_rx
	( 
	mailbox_type& letter 
	)
{
if( destination != current_location )
	return;
}


template <int M>
void add_to_transmit_queue
	(
		void
	)
{

}
template <int M>
void core::mailbox::transmit_engine
	(
	void
	)
{

this->lora_pack_engine(); //do something w/ p_transmit_queue and make it MessageAPI compatable, need to sort into destination buckets, or make messageAPI have an ALL units ID?
//send packed
for( )
//wait for ack
//update msgAPI Key

}

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
letter_name letter_index;

/*----------------------------------------------------------
Local constants
----------------------------------------------------------*/
const std::unordered_map< data_type, int > data_size_map = { { FLOAT_32_TYPE : 4 }
															 { UINT_32_TYPE  : 4 },
															 { BOOLEAN_TYPE :  1 } };

/*----------------------------------------------------------
Init variables
----------------------------------------------------------*/
memset( &return_msg, 0, sizeof( tx_message ) );
return_msg.destination = MODULE_NONE;
message_full           = false;
current_index          = 0;
data_size              = 0;
letter_index           = LETTER_NONE;

/*----------------------------------------------------------
loop untill Tx queue is empty or tx_message is full
----------------------------------------------------------*/
while( p_transmit_queue.size() > 0 || message_full )
	{
	/*------------------------------------------------------
	loop untill Tx queue is empty or tx_message is full
	------------------------------------------------------*/
	letter_index   = p_transmit_queue.front();
	mailbox_type& current_letter = p_mailbox_ref[ letter_index ];

	data_size = data_size_map[ current_letter.type ];

	if( current_index + data_size + 1 > MAX_LORA_MSG_SIZE )
		{
		message_full = true;
		continue;
		}

	//place 

	//memcopy
	memcpy( tx_message.message[current_index], letter.data, data_size );

	current_index += data_size;

	p_transmit_queue.pop();
	}

return tx_message;
}




/*
more thoughts

1) table should be global accross units, which means terms like source and destination, make sense? no dual communication
2) TX should happen every 1s to avoid having lora in TX too long
3) TX should pack using the transmit queue and transmit engine
4) does update rate make sense or should it just always be async? not sure on this one 


*/


