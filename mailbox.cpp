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
#include <unordered_map>

/*--------------------------------------------------------------------
                          GLOBAL NAMESPACES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                          LITERAL CONSTANTS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                                TYPES
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
mailbox()
--------------------------------------------------------------------*/
/*********************************************************************
*
*   PROCEDURE NAME:
*       mailbox::mailbox (constructor)
*
*   DESCRIPTION:
*       mailbox class constructor
*
*********************************************************************/
mailbox::mailbox( mailbox_type& global_mailbox[], , int mailbox_size );
    {
    p_mailbox_size = mailbox_size;
    p_mailbox_ref = global_mailbox;
    p_internal_clk = 0;
    }

/*********************************************************************
*
*   PROCEDURE NAME:
*       mailbox::~mailbox (deconstructor)
*
*   DESCRIPTION:
*       mailbox class deconstructor
*
*********************************************************************/
mailbox::~mailbox( void ) 
	{
	}



/*********************************************************************
*
*   PROCEDURE NAME:
*       mailbox::mailbox_runtime()
*
*   DESCRIPTION:
*       this to be run every 100ms (fastest update rate) to handle
*       different mailbox items
*
*********************************************************************/
void mailbox::mailbox_runtime( void )
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
P_transmit_queue = {}; //clear for processing loop 
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
			//console assert 
			break;
		}

	
    }

//update clock
p_internal_clk = ( p_internal_clk + 100 ) % 1000;

} /* mailbox:mailbox_runtime() */


void mailbox::process_tx
	( 
	mailbox_type& letter 
	)
{

}


void mailbox::process_rx
	( 
	mailbox_type& letter 
	)
{

}



void add_to_transmit_queue
	(
		void
	)
{

}

void mailbox::transmit_engine
	(
	void
	)
{

}