# Design

For high-performance I/O, we use linux io_uring and windows IOCP.
Therefore, any recv call provides preallocated memory, which caller must return after use.
On the other hand, any send_prep call provides preallocated memory, which the caller writes into.
