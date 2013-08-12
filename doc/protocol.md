# Protocol
This document describes the different packets that are used for communication between nodes. Cryptography is done using [Elliptic Curve Cryptography][ecc] (ECC) with 256 bit keys. The choice for ECC is mainly due to the smaller key sizes, leading to less overhead. The suspicion that RSA may soon be severely weakened or broken also plays a role.

IPv6/IPv4 header (40 bytes)
UDP header (8 bytes)
LSDRP header:
- version (1 byte) - if there is no explicit support for this version the packet should be dropped
- header length (1 byte) - in 8 byte units
- message type (1 byte) - message type
- hop limit (1 byte) - hop limit
- flags (1 byte) - bit 1: is payload encrypted?
- flow id (3 bytes) - used for routing swap packets

- source pubkey (256 bits/32 bytes)
- destination pubkey (256 bits/32 bytes)
- source location (128 bit/16 bytes)
- destination location (128 bit/16 bytes)

- Signature using source privkey (of entire packet with signature field zero, 64 bytes)

- Message contents possibly encrypted with destination pubkey (only encrypted, not signed)

Note: The first byte of a location is reserved for the location type for future experiments and should be zero.

The header (pubkeys + location) could optionally be encrypted but this makes plain switching duties much more expensive.

### Message 0: data packet
- Encrypted payload:
- Original IPv6 packet with source and destination address removed (can be derived from pubkeys)
- The total overhead is: 40 + 8 + 4 + 32*2 + 16*2 + 64 -2*32= 148 bytes overhead (ipsec = 56)

### Message 1: swap request
- source information filled
- destination information zero -> randomly routed
- hop limit around 6
- Payload: optimal location (128 bits)

### Message 2: swap response
- Drop if your location is not closer to the optimal location from the requester
- Note: resend until you receive a confirmation, then perform the swap
- Payload: optimal location (128 bits)

### Message 3: swap confirmation
- No payload

### Message 4: location query
- Note: destination location is the hash of the address

### Message 5: location answer
- Unencrypted payload
- Pubkey
- Location
- Timestamp (64 bit UTC)
- Signature
- Extra fields
Note: can be used for inserts with zero address, with as destination location the hashed address

### Later:
Message 6: Ping
- Can be sent to the zero address (to only a location)
- Can be used to check if a location is in use

Message 7: Pong

Message 8: Dead-end
Message 9: HTL exceeded
Message 10: Invalid packet

### Receiving a packet
- Invalid packets (Type 1, destination address not zero) -> drop
- If type is 1 or 2 add address + flow id to flow table
- HTL 1, type 1, destination address zero -> process swap request
- Type 1, destination address zero -> switching
- Type 5 -> store location if newer
- We are not the destination -> switching

- Type 0 -> send to local interface
- Type 2 -> process swap response
- Type 3 -> process swap confirmation
- Type 4 -> answer if possible, otherwise switch
- Type 5, we are the destination -> drop silently (already processed)

- Log error

#### Switching
- Decrease HTL, drop if 0
- Destination address zero, location zero -> route to random other peer
- Destination address nonzero and in flow table -> forward to peer from table
- route to peer with closest location

[ecc]: http://en.wikipedia.org/wiki/Elliptic_curve_cryptography "Elliptic Curve Cryptography"

