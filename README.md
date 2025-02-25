# Mailbox API
Mailbox API is a messaging interface built upon LoraAPI and messageAPI


# INTAGRATION NOTE

rough draft of changes w/ rx multi and rounds, still needs cleanup

# Data Formatting
This is how data is formatted in the 10 byte MessageAPI data buffer

| Type | Data Format|
| --- | --- |
| Ack  | [Ack ID] [ Data to be Ack'ed Index] |
| Round Update  | [Rnd Update ID ] [ New Round Value ] |
| Data | [Index] [ Data ... ]  |

## RX Route
- rx_runtime() 
    - gets all messages since last run
    - Calls lora_unpack_engine()
        - go through raw lora stream and convert into __msgAPI_rx__
        - add msgAPI_rx object to __p_rx_queue__
    - if __p_rx_queue__ is not empty
        - for data: 
            - process_rx_data()
                - get global mailbox reference
                - updata data and update flag
            - push ack to __p_transmit_queue__
        - for ack: update ack_awaiting
        - for update: update round 
## TX Route
- tx_runtime()
    - if current round != current unit, skip
    - go through entire global mailbox:
        - skip RX
        - if update rate/round matches, call process_tx():
            - determine based upon async or not to add to __p_transmit_queue__
    - update round counter & add round update to __p_transmit_queue__
    - run transmit_engine():
        - verify acks from previous rounds
        - while p_transmit_queue is not empty:
            - lora_pack_engine():
                - run through queue and generate packed lora messages for messageAPI payloads. 
            - messageAPI send over lora



# 