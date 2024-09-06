# Mailbox API
Mailbox API is a messaging interface built upon LoraAPI and messageAPI


# INTAGRATION NOTE

from reading the RMF95 ICD (pg 36) it looks like payload information is ONLY available until the next transmission?

"In continuous mode status information are available only for the last packet received, i.e. the corresponding registers should be read before the next RxDone arrives."

need to verify if maybe I need to make a special mutli_msg_tx format? need to test this


additionally I had issues when testing where one lru sends something while another one is... thus the final message is a mix of both. I think i need to add a "rounds" system where each lryu takes turns tx'ing.
