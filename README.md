# Build Requirements

### Windows

* llvm-gcc version 22.1.8
* cmake version 4.3.4
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

6. Receive packet. Identify remote from its UDP 4 tuple
