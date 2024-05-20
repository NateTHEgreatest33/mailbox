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
core::mailbox::mailbox( mailbox_type& global_mailbox[], , int mailbox_size );
    {
	
	std::static_assert( mailbox_size > MAX_SIZE_GLOBAL_MAILBOX);
	//create max msg length (max unit8 size = 256?) and static assert the size of mailbox... or template typename T
    p_mailbox_size = mailbox_size;
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
core::mailbox::~mailbox( void ) 
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
void core::mailbox::mailbox_runtime( void )
{
int i;
bool process;
const std::unordered_map< update_rate, std::function<bool(int)> > process_map = { { 100MS :  [](int clk ){ return true;  }                                      },
																		 		  { 500_MS : [](int clk ){ return ( clk == 0 || clk == 500 ) ? true : false;  } },
																				  { 1_S :    [](int clk ){ return ( clk == 0 );  }                              },
																				  { ASYNC :  [](int clk ){ return true;  }                                      }
                                                                                };
i       = 0;
process = false;
p_transmit_queue = {}; //clear for processing loop 
//foreach item in global mailbox
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


void core::mailbox::process_tx
	( 
	mailbox_type& letter 
	)
{
if( source != current_unit )
	return;

//add to transmit queue if needing to be transmitted. thinking 
}


void core::mailbox::process_rx
	( 
	mailbox_type& letter 
	)
{
if( destination != current_location )
	return;
}



void add_to_transmit_queue
	(
		void
	)
{

}

void core::mailbox::transmit_engine
	(
	void
	)
{

this->pack_engine(); //do something w/ p_transmit_queue and make it MessageAPI compatable, need to sort into destination buckets, or make messageAPI have an ALL units ID?
//send packed
for( )
//wait for ack
//update msgAPI Key

}

//wont empty will just create 
tx_message core::mailbox::pack_engine
	(
	void
	)
{
//consider making a priority queue for higher priority messages?
tx_message return_msg;
//memset 
tx_tr = 0;
for( auto letter : p_transmit_queue )
	{
	auto current_letter = letter.top();

	if(  size_map[ current_letter.type ] + tx_tr >=  MAX_LORA_SIZE )
		{
		break; //reached the end of the sizing 
		}

	return_msg[tx_itr++] = 0;//need to figure out index, maybe queue should be index's?
	tx_tr += size_map[ current_letter.type ];

	}

//we have re


//messageAPI only allows 10 bytes to be transmited at time, this means 


}




/*
more thoughts

1) table should be global accross units, which means terms like source and destination, make sense? no dual communication
2) TX should happen every 1s to avoid having lora in TX too long
3) TX should pack using the transmit queue and transmit engine
4) does update rate make sense or should it just always be async? not sure on this one 


*/


