#ifndef MAILBOX_MAP_HPP
#define MAILBOX_MAP_HPP
/*********************************************************************
*
*   HEADER:
*       system definitions for mailbox API. enum mbx_index must match
*       global_mailbox in order for mailbox API to function properly.
*
*   Copyright 2025 Nate Lenze
*
*********************************************************************/

/*--------------------------------------------------------------------
                           GENERAL INCLUDES
--------------------------------------------------------------------*/
#include "mailbox_types.hpp"
#include "mailbox_map_types.hpp"

#include "sys_def.h"

#include <array>

/*--------------------------------------------------------------------
                           MEMORY CONSTANTS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                          LITERAL CONSTANTS
--------------------------------------------------------------------*/
#define TESTING (true)
/*--------------------------------------------------------------------
                                TYPES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                           MEMORY CONSTANTS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                              VARIABLES
--------------------------------------------------------------------*/
#ifndef TESTING
std::array<mailbox_type, (size_t)mbx_index::NUM_MAILBOX > global_mailbox
{{
/* data, type,                     updt_rt,                  flag,               direction,     destination, source       */
{ 0,     data_type::UINT_32_TYPE,  update_rate::RT_ASYNC,    flag_type::NO_FLAG, direction::RX, RPI_MODULE,  PICO_MODULE },  /* EXAMPLE_INT_MSG */
{ 0.0f,  data_type::FLOAT_32_TYPE, update_rate::RT_1_ROUND,  flag_type::NO_FLAG, direction::TX, PICO_MODULE, RPI_MODULE  },  /* EXAMPLE_FLT_MSG */
{ 0,     data_type::UINT_32_TYPE,  update_rate::RT_5_ROUND,  flag_type::NO_FLAG, direction::RX, PICO_MODULE, RPI_MODULE  },  /* EXAMPLE_RX_MSG  */
{ 0.0f,  data_type::FLOAT_32_TYPE, update_rate::RT_5_ROUND,  flag_type::NO_FLAG, direction::RX, PICO_MODULE, RPI_MODULE  }   /* EXAMPLE_FLT_RX_MSG  */

}};
#else
std::array<mailbox_type, (size_t)mbx_index::NUM_MAILBOX > global_mailbox
{{
/* data, type,                     updt_rt,                  flag,               direction,     destination, source       */
{ 0.0,   data_type::FLOAT_32_TYPE, update_rate::RT_1_ROUND,  flag_type::NO_FLAG, direction::RX, PICO_MODULE, RPI_MODULE  },  /* FLOAT_TX_FROM_RPI_MSG */
{ 0.0,   data_type::FLOAT_32_TYPE, update_rate::RT_1_ROUND,  flag_type::NO_FLAG, direction::TX, RPI_MODULE,  PICO_MODULE },  /* FLOAT_RX_FROM_RPI_MSG */
{ 0,     data_type::UINT_32_TYPE,  update_rate::RT_1_ROUND,  flag_type::NO_FLAG, direction::RX, PICO_MODULE, RPI_MODULE  },  /* INT_TX_FROM_RPI_MSG   */
{ 0,     data_type::UINT_32_TYPE,  update_rate::RT_1_ROUND,  flag_type::NO_FLAG, direction::TX, RPI_MODULE,  PICO_MODULE },  /* INT_RX_FROM_RPI_MSG   */
{ false, data_type::BOOLEAN_TYPE,  update_rate::RT_1_ROUND,  flag_type::NO_FLAG, direction::RX, PICO_MODULE, RPI_MODULE  },  /* BOOL_TX_FROM_RPI_MSG  */
{ false, data_type::BOOLEAN_TYPE,  update_rate::RT_1_ROUND,  flag_type::NO_FLAG, direction::TX, RPI_MODULE,  PICO_MODULE },  /* BOOL_RX_FROM_RPI_MSG  */
{ 0,     data_type::UINT_32_TYPE,  update_rate::RT_ASYNC,    flag_type::NO_FLAG, direction::RX, PICO_MODULE, RPI_MODULE  },  /* ASYNC_TX_FROM_RPI_MSG */
{ 0,     data_type::UINT_32_TYPE,  update_rate::RT_ASYNC,    flag_type::NO_FLAG, direction::TX, RPI_MODULE,  PICO_MODULE },  /* ASYNC_RX_FROM_RPI_MSG */
{ 0,     data_type::UINT_32_TYPE,  update_rate::RT_5_ROUND,  flag_type::NO_FLAG, direction::RX, PICO_MODULE, RPI_MODULE  },  /* RND_5_TX_FROM_RPI_MSG */
{ 0,     data_type::UINT_32_TYPE,  update_rate::RT_5_ROUND,  flag_type::NO_FLAG, direction::TX, RPI_MODULE,  PICO_MODULE },  /* RND_5_RX_FROM_RPI_MSG */
{ 0,     data_type::UINT_32_TYPE,  update_rate::RT_ASYNC,    flag_type::NO_FLAG, direction::RX, PICO_MODULE, RPI_MODULE  },  /* TEST_TX_FROM_RPI_MSG  */
{ 0,     data_type::UINT_32_TYPE,  update_rate::RT_ASYNC,    flag_type::NO_FLAG, direction::TX, RPI_MODULE,  PICO_MODULE }   /* TEST_RX_FROM_RPI_MSG  */
}};
#endif
/*--------------------------------------------------------------------
                                MACROS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                              PROCEDURES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                           Enum Verification
--------------------------------------------------------------------*/
static_assert( global_mailbox.size() == static_cast<std::size_t>(mbx_index::NUM_MAILBOX), "mbx_index enum is not correctly sized to mailbox size" );


/* mailbox_map.hpp */
#endif