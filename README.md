# Mailbox API Documentation

## Table of Contents
- [Overview](#overview)
- [Template Parameters](#template-parameters)
- [Public Interface](#public-interface)
  - [Constructor](#constructor)
  - [rx_runtime](#rx_runtime)
  - [tx_runtime](#tx_runtime)
  - [update](#update)
  - [access](#access)
  - [watchdog](#watchdog)
- [Example Usage](#Example-Usage)
  - [Message Loop](#message-loop)
  - [Message Packing and Unpacking](#message-packing-and-unpacking)
  - [Message Types](#message-types)
- [Key Data Structures](#key-data-structures)
- [Configuration Constants](#configuration-constants)
- [Dependencies](#dependencies)

---

## Overview

The `core::mailbox` class provides a templated, thread-safe mechanism for inter-module communication within the Smart-Home-Core ecosystem. It is designed for both synchronous and asynchronous data and abstracts the finer details away from the user. 

It manages a shared "mailbox," which is an array of `mailbox_type` objects. Each slot in the mailbox represents a specific data point that can be written to (transmitted) or read from (received). The class handles message serialization (packing), deserialization (unpacking), acknowledgments (ACKs), and a time-slot based transmission schedule to ensure fair bus access for all modules.

## Dependencies
While the mailbox system is a standalone API. it relies *heavly* on the messageAPI and LoraAPI for the underlying communication. Addition for debugging the ConsoleAPI is used, and for thread saftey, the utility mutex wrapper functions are used.
- [LoraAPI](https://github.com/NateTHEgreatest33/LoRa)
- [MesageAPI](https://github.com/NateTHEgreatest33/messageAPI)
- [ConsoleAPI](https://github.com/NateTHEgreatest33/console_api)
- [Util](https://github.com/NateTHEgreatest33/utilities)

## Template Parameters
### `template <int M>`
-   **`M`**: An integer representing the total number of entries in the global mailbox array. This size and array must be consistent across all modules using the mailbox system.

---

## Public Interface

### Constructor

```cpp
core::mailbox<M>::mailbox(std::array<mailbox_type, M>& global_mailbox);
```
The constructor initializes the mailbox instance.

-   **Parameters:**
    -   `global_mailbox`: A reference to the global static array of `mailbox_type` entries that defines the entire system's shared data map.


### `rx_runtime`

```cpp
void core::mailbox<M>::rx_runtime(void);
```
The primary message receiving and processing loop. This function is to be called alongside `tx_runtime()`.


### `tx_runtime`

```cpp
void core::mailbox<M>::tx_runtime(void);
```
The primary message transmission loop. This function should be called on a regular, timed interval (e.g., every 100ms), alongside `rx_runtime()`. The rate of this function call determines the speed of each "round"

### `update`

```cpp
bool core::mailbox<M>::update(data_union d, int global_mbx_indx, bool user_mode);
```
A thread-safe public method for application code to write data to a mailbox entry.

-   **Parameters:**
    -   `d`: A `data_union` containing the data to be written.
    -   `global_mbx_indx`: The integer index of the mailbox entry to update.
    -   `user_mode`: A boolean indicating the context of the call. `true` for application-level writes (TX), `false` for internal updates from the mailbox engine itself. 
-   **Returns:** `true` on success, `false` if the index is invalid or the direction is incorrect (e.g., application trying to write to an `RX` slot).


### `access`

```cpp
data_union core::mailbox<M>::access(mbx_index global_mbx_indx, flag_type& current_flag, bool clear_flag = true);
```
A thread-safe public method for application code to read data from a mailbox entry.

-   **Parameters:**
    -   `global_mbx_indx`: The index of the mailbox entry to access.
    -   `current_flag` (out): A reference that will be populated with the entry's current flag (`NO_FLAG`, `RECEIVE_FLAG`, etc.).
    -   `clear_flag`: If `true` (default), the entry's flag is reset to `NO_FLAG` after the access.
-   **Returns:** A `data_union` containing the data from the mailbox entry.

### `watchdog`

```cpp
void core::mailbox<M>::watchdog(void);
```
A safety function to prevent the system from stalling if a module fails to transmit. If `tx_runtime` hasn't run (and thus hasn't "pet" the watchdog), this function will force the current module to take over the transmit round. This should be run at a rate greater than `tx_runtime()`.

---

## Example Usage

### Setup a global mailbox & type
```cpp
std::array<mailbox_type, (size_t)mbx_index::NUM_MAILBOX > global_mailbox
{{
/* data, type,                     updt_rt,                  flag,               direction,     destination, source       */
{ 0,     data_type::UINT_32_TYPE,  update_rate::RT_ASYNC,    flag_type::NO_FLAG, direction::RX, RPI_MODULE,  PICO_MODULE },  /* EXAMPLE_INT_MSG */
{ 0.0f,  data_type::FLOAT_32_TYPE, update_rate::RT_1_ROUND,  flag_type::NO_FLAG, direction::TX, PICO_MODULE, RPI_MODULE  },  /* EXAMPLE_FLT_MSG */
{ 0,     data_type::UINT_32_TYPE,  update_rate::RT_5_ROUND,  flag_type::NO_FLAG, direction::RX, PICO_MODULE, RPI_MODULE  },  /* EXAMPLE_RX_MSG  */
{ 0.0f,  data_type::FLOAT_32_TYPE, update_rate::RT_5_ROUND,  flag_type::NO_FLAG, direction::RX, PICO_MODULE, RPI_MODULE  }   /* EXAMPLE_FLT_RX_MSG  */
}};
```
```cpp
enum struct mbx_index : uint8_t
    {
    EXAMPLE_INT_MSG,       /* Example Int Msg   */
    EXAMPLE_FLT_MSG,       /* Example flt Msg   */
    EXAMPLE_RX_MSG,        /* Example rx Msg    */
    EXAMPLE_FLT_RX_MSG,    /* Example flt rx 
                                            Msg */

    NUM_MAILBOX,           /* Number of Mailbox */
    MAILBOX_NONE,          /* Mailbox None      */
    
    RESERVED_1 = 0xFF,     /* ACK ID            */
    RESERVED_2 = 0xFE      /* Round Update ID   */
    
    };
```

### Initilize mailboxAPI
```cpp
core::mailbox< (size_t)mbx_index::NUM_MAILBOX > Mailbox( global_mailbox );
```

### Call TX and RX Runtime
```cpp
/*------------------------------------------------------
Run mailbox rx periodic
------------------------------------------------------*/
Mailbox.rx_runtime();

/*------------------------------------------------------
Run mailbox tx periodic every 100ms
------------------------------------------------------*/
if( s_current_100ms_timeout < get_absolute_time() )
    {
    Mailbox.tx_runtime();
    s_current_100ms_timeout = delayed_by_ms( get_absolute_time(), 100 );
    }

```

### Pet watchdog
```cpp
Mailbox.watchdog();
```

### Access/Update data

```cpp
// Update a mailbox entry (write)
Mailbox[mbx_index::EXAMPLE_INT_MSG] = return_value;

// Access a mailbox entry (read)
flag_type temp_flag;
temp_data = Mailbox[mbx_index::EXAMPLE_RX_MSG].access_with_flag(temp_flag);

// If you don't need the flag, you can read directly:
data_union simple_data = Mailbox[mbx_index::EXAMPLE_RX_MSG];
```