# Build Requirements

### Commons

* cppcheck
* clang (llvm-gcc) version 22.1.8
* cmake version 4.3.4

* Keep line width under 80

### Windows

* windows SDK

### Ubuntu


# Architecture

A virtual network adapter participates in a mesh network.
UDP-based, SWIM for dead peer detection.

Aims for minimum overhead, with integrated security and peer discovery.
AES-GCM, encrypted payload.

[Tx]
1. Socket binds to virtual network adapter
2. Socket sends packet
3. Extract flags, remove unnecessary fields
4. Encrypt packet
5. Send over UDP. Minimum payload size: 32 + 28 byte

**Important**

First 1 bit of the UDP payload determines whether the payload is a tunneled packet or a control packet.
If it is a control packet, the following 7 bits determine the type of control packet.
0: chat (no semantics, just asynchronous text exchange for debug/log.)

[Rx]
1. Receive packet. Identify remote from its UDP 4 tuple
2. If the packet is from a known host, try decrypt with the known shared key
3. If successfully decrypts, transfer packet
4. If decryption fails, drops packet
5. If the packet is from an unknown host, 

[Keep-Alive]
1. Periodically (30 sec) scan for inactive peers
2. Select 3 peers among unactive peers, send ICMP ping (under tunnel)
3. If ping loss, consider it as dropped
4. Acknowledge peer down to others
5. 