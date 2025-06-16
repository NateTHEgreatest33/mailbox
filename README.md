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
- [Internal Mechanics](#internal-mechanics)
  - [Message Loop](#message-loop)
  - [Message Packing and Unpacking](#message-packing-and-unpacking)
  - [Message Types](#message-types)
- [Key Data Structures](#key-data-structures)
- [Configuration Constants](#configuration-constants)
- [Dependencies](#dependencies)

---

## Overview

The `core::mailbox` class provides a templated, thread-safe mechanism for inter-module communication within the Smart-Home-Core ecosystem. It is designed to abstract the complexities of a message-passing system, likely over a protocol like Loora (as suggested by function names like `lora_pack_engine`).

It manages a shared "mailbox," which is an array of `mailbox_type` objects. Each slot in the mailbox represents a specific data point that can be written to (transmitted) or read from (received). The class handles message serialization (packing), deserialization (unpacking), acknowledgments (ACKs), and a round-robin based transmission schedule to ensure fair bus access for all modules.

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
-   **Functionality:**
    -   Stores a reference to the global mailbox.
    -   Initializes mutexes for thread-safe access.
    -   Resets internal state counters and flags.

### `rx_runtime`

```cpp
void core::mailbox<M>::rx_runtime(void);
```
The primary message receiving and processing loop. This function should be called as frequently as possible in the main application loop to ensure timely message handling.

-   **Functionality:**
    -   Polls the `messageAPI` for new incoming raw messages.
    -   If a message is received, it passes it to the `lora_unpack_engine` to be deserialized.
    -   Processes the unpacked messages from an internal receive queue (`p_rx_queue`).
    -   Handles incoming `data`, `ack`, and `update` messages appropriately. For data messages, it updates the corresponding mailbox entry and queues an ACK for transmission.

### `tx_runtime`

```cpp
void core::mailbox<M>::tx_runtime(void);
```
The primary message transmission loop. This function should be called on a regular, timed interval (e.g., every 100ms).

-   **Functionality:**
    -   Only executes if it's the current module's turn to transmit, determined by `p_transmit_round`.
    -   Iterates through the global mailbox to find `TX` entries that are due for an update based on their `update_rate`.
    -   Queues necessary data transmissions and a round-update message.
    -   Calls the `transmit_engine` to pack and send the queued messages.
    -   Pets an internal watchdog flag (`p_watchdog_pet`).

### `update`

```cpp
bool core::mailbox<M>::update(data_union d, int global_mbx_indx, bool user_mode);
```
A thread-safe public method for application code to write data to a mailbox entry.

-   **Parameters:**
    -   `d`: A `data_union` containing the data to be written.
    -   `global_mbx_indx`: The integer index of the mailbox entry to update.
    -   `user_mode`: A boolean indicating the context of the call. `true` for application-level writes (TX), `false` for internal updates from received messages (RX).
-   **Returns:** `true` on success, `false` if the index is invalid or the direction is incorrect (e.g., application trying to write to an `RX` slot).
-   **Functionality:**
    -   Acquires a mutex lock.
    -   Updates the data and sets the appropriate flag (`TRANSMIT_FLAG` or `RECEIVE_FLAG`).

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
-   **Functionality:**
    -   Acquires a mutex lock.
    -   Retrieves the data and flag.
    -   Optionally clears the flag.

### `watchdog`

```cpp
void core::mailbox<M>::watchdog(void);
```
A safety function to prevent the system from stalling if a module fails to transmit. If `tx_runtime` hasn't run (and thus hasn't "pet" the watchdog), this function will force the current module to take over the transmit round.

---

## Internal Mechanics

### Message Loop
The mailbox operates on two distinct runtimes:
1.  **`rx_runtime`**: A fast loop that continuously checks for incoming messages.
2.  **`tx_runtime`**: A slower, periodic loop that handles the transmission of data based on a round-robin schedule. This prevents any single module from monopolizing the communication bus.

### Message Packing and Unpacking
-   **`lora_pack_engine`**: Collects messages from the transmit queue (`p_transmit_queue`) and packs them into a single `tx_message` frame. It handles different message types and determines the final destination of the packet (a specific module or all modules).
-   **`lora_unpack_engine`**: Takes a raw `rx_multi` message and parses it. It iterates through the packed data, identifying each piece by its ID (e.g., `MSG_ACK_ID`, `MSG_UPDATE_ID`, or a mailbox index) and places the deserialized data into the receive queue (`p_rx_queue`).

### Message Types
The communication protocol defines three primary message types that are packed into a single physical transmission:
1.  **Data**: A standard data payload, identified by its mailbox index.
2.  **ACK (`MSG_ACK_ID`)**: An acknowledgment for a received data message, identified by the mailbox index it is acknowledging.
3.  **Update (`MSG_UPDATE_ID`)**: A message broadcast to all modules to signal that the transmit round has advanced to the next module.

---

## Key Data Structures
-   [`mailbox_type`](mailbox_types.hpp:1): The core struct defining a single entry in the mailbox. It contains metadata like direction (TX/RX), source/destination, data type, update rate, and the data itself.
-   [`data_union`](mailbox_types.hpp:1): A union that can hold different data types (`float`, `uint32_t`, etc.) in the same memory location.
-   [`msgAPI_tx`](messageAPI/messageAPI.hpp:1) / [`msgAPI_rx`](messageAPI/messageAPI.hpp:1): Structs used for internal queueing of transmit and receive requests.

---

## Configuration Constants
-   `MAX_SIZE_GLOBAL_MAILBOX (254)`: The maximum number of entries allowed in the mailbox.
-   `MSG_ACK_ID (0xFF)`: The byte identifier for an acknowledgment message.
-   `MSG_UPDATE_ID (0xFE)`: The byte identifier for a round-update message.

---

## Dependencies
-   [`messageAPI.hpp`](lib/messageAPI/messageAPI.hpp:1): The low-level message sending/receiving interface.
-   [`mailbox_types.hpp`](lib/mailbox/mailbox_types.hpp:1): Contains core type definitions for the mailbox system.
-   [`Console.hpp`](lib/console/console.hpp:1): Used for logging asserts and errors.
-   [`queue.hpp`](lib/util/queue.hpp:1): A utility for the internal FIFO queues.
-   [`mutex_lock.hpp`](lib/util/mutex_lock.hpp:1): A utility for thread-safe mutex locking.