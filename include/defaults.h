#ifndef DEFAULTS_H
#define DEFAULTS_H

#define MAX_PACKET_SIZE 65536
#define ADAPTER_NAME_MAX_LENGTH 64

#define HEX_BYTES_PER_LINE 16
#define HEX_BYTES_LINE_LENGTH HEX_BYTES_PER_LINE * 4

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif // MIN

// performance tuning parameters
// IOCP / io_uring
#define NUM_RECVS 256
#define RING_BUFFER_SIZE 1500 * NUM_RECVS

#endif // DEFAULTS_H