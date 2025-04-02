
/*********************************************************************
*
*   NAME:
*       mailbox_test.cpp
*
*   DESCRIPTION:
*       Testing functionality for Mailbox system test
*
*   Copyright 2025 Nate Lenze
*
*********************************************************************/

/*--------------------------------------------------------------------
                              INCLUDES
--------------------------------------------------------------------*/
#include "mailbox.hpp"
#include "mailbox_test.hpp"
#include "mailbox_types.hpp"

#include "mailbox_map_types.hpp"

#include <unordered_map>
#include <cstring>
/*--------------------------------------------------------------------
                          GLOBAL NAMESPACES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                          LITERAL CONSTANTS
--------------------------------------------------------------------*/
#define NUM_TX_TEST_CASES (5)
#define NUM_RX_TEST_CASES (5)

/*--------------------------------------------------------------------
                                TYPES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                           MEMORY CONSTANTS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                              VARIABLES
--------------------------------------------------------------------*/
const std::unordered_map< uint32_t, std::pair<mbx_index, data_union> > rx_test_cases = 
{
{ 20, { mbx_index::FLOAT_RX_FROM_RPI_MSG, data_union{5.5}   } },
{ 21, { mbx_index::INT_RX_FROM_RPI_MSG,   {.integer = 5}    } },
{ 22, { mbx_index::BOOL_RX_FROM_RPI_MSG,  {.boolean = true} } },
{ 23, { mbx_index::ASYNC_RX_FROM_RPI_MSG, {.integer = 10}   } },
{ 24, { mbx_index::RND_5_RX_FROM_RPI_MSG, {.integer = 15}   } }
};

const std::unordered_map< uint32_t, std::pair<mbx_index, data_union> > tx_test_cases = 
{
/* return data     {index, expected data} */
{ 10, { mbx_index::FLOAT_TX_FROM_RPI_MSG, data_union{5.5}   } },
{ 11, { mbx_index::INT_TX_FROM_RPI_MSG,   data_union{5}    } },
{ 11, { mbx_index::INT_TX_FROM_RPI_MSG,   {.integer = 5}    } },
{ 12, { mbx_index::BOOL_TX_FROM_RPI_MSG,  {.boolean = true} } },
{ 13, { mbx_index::ASYNC_TX_FROM_RPI_MSG, {.integer = 10}   } },
{ 14, { mbx_index::RND_5_TX_FROM_RPI_MSG, {.integer = 15}   } }
};


/*--------------------------------------------------------------------
                                MACROS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                                EXTERNS
--------------------------------------------------------------------*/
extern core::mailbox< (size_t)mbx_index::NUM_MAILBOX > Mailbox;

/*--------------------------------------------------------------------
                              PROCEDURES
--------------------------------------------------------------------*/
/*********************************************************************
*
*   PROCEDURE NAME:
*       run()
*
*   DESCRIPTION:
*       testing runtime
*
*********************************************************************/
void mailbox_testing::run
    ( 
    void
    )
{
/*----------------------------------------------------------
Local variables
----------------------------------------------------------*/
int i;
auto tx_itr = tx_test_cases.begin();
auto rx_itr = rx_test_cases.begin();
data_union return_value;
data_union temp_data;
flag_type temp_flag;
/*----------------------------------------------------------
Initialize variables
----------------------------------------------------------*/
return_value.integer = 0xFF;
/*----------------------------------------------------------
TX (from raspberry pi) test cases
----------------------------------------------------------*/
while( tx_itr != tx_test_cases.end() )
    {
    
    /* access at mailbox index */
    temp_data = Mailbox.access( (tx_itr->second).first, temp_flag );

    /* if data has been updated since last read */
    if( temp_flag != flag_type::NO_FLAG )
        {
        
        /* if value matches expected value  */
        if( memcmp( &((tx_itr->second).second), &temp_data, sizeof(data_union) ) == 0 )
        // if( (tx_itr->second).second == temp_data )
            {
            return_value.integer = tx_itr->first;
            Mailbox.update( return_value, static_cast<int>(mbx_index::TEST_RX_FROM_RPI_MSG ) );//this needs to be updated
            }

        }

    tx_itr++;
    }


/*----------------------------------------------------------
clear data
----------------------------------------------------------*/
//memset return_value, temp_data, temp_flag

/*----------------------------------------------------------
RX (to raspberry pi) test cases
----------------------------------------------------------*/
while( rx_itr != rx_test_cases.end() )
    {
    /* get test command */
    temp_data = Mailbox.access( mbx_index::TEST_TX_FROM_RPI_MSG, temp_flag );

    //check if tenp_data exists in map - may need to use itr, i forget if thats the case
    auto pair = rx_test_cases.find( temp_data.integer );

    if( pair != rx_test_cases.end() )
        {
        Mailbox.update( (pair->second).second, static_cast<int>((pair->second).first)  );
        }

    rx_itr++;
    }

}