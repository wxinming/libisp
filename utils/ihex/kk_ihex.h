#ifndef KK_IHEX_H
#define KK_IHEX_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef IHEX_USE_STDBOOL
#include <stdbool.h>
typedef bool ihex_bool_t;
#else
typedef uint_fast8_t ihex_bool_t;
#endif

typedef uint_least32_t ihex_address_t;
typedef uint_least16_t ihex_segment_t;
typedef int ihex_count_t;

// Maximum number of data bytes per line (applies to both reading and
// writing!); specify 255 to support reading all possible lengths. Less
// can be used to limit memory footprint on embedded systems, e.g.,
// most programs with IHEX output use 32.
#ifndef IHEX_LINE_MAX_LENGTH
#define IHEX_LINE_MAX_LENGTH 255
#endif

#define IHEX_BUFFER_SIZE 1024 * 1024

enum ihex_flags {
    IHEX_FLAG_ADDRESS_OVERFLOW = 0x80   // 16-bit address overflow
};
typedef uint8_t ihex_flags_t;

typedef struct ihex_state {
    ihex_address_t  address;
#ifndef IHEX_DISABLE_SEGMENTS
    ihex_segment_t  segment;
#endif
    ihex_flags_t    flags;
    uint8_t         type;
    uint8_t         line_length;
    uint8_t         length;
	uint8_t         data[IHEX_LINE_MAX_LENGTH + 1];
    struct ihex_buffer {
        uint8_t* data;
        uint32_t size;
        uint32_t heap_size;
    } buffer;
    uint32_t        line_number;
} kk_ihex_t;

enum ihex_record_type {
    IHEX_DATA_RECORD,
    IHEX_END_OF_FILE_RECORD,
    IHEX_EXTENDED_SEGMENT_ADDRESS_RECORD,
    IHEX_START_SEGMENT_ADDRESS_RECORD,
    IHEX_EXTENDED_LINEAR_ADDRESS_RECORD,
    IHEX_START_LINEAR_ADDRESS_RECORD
};
typedef uint8_t ihex_record_type_t;

#ifndef IHEX_DISABLE_SEGMENTS

// Resolve segmented address (if any). It is the author's recommendation that
// segmented addressing not be used (and indeed the write function of this
// library uses linear 32-bit addressing unless manually overridden).
//
#define IHEX_LINEAR_ADDRESS(ihex) ((ihex)->address + (((ihex_address_t)((ihex)->segment)) << 4))
//
// Note that segmented addressing with the above macro is not strictly adherent
// to the IHEX specification, which mandates that the lowest 16 bits of the
// address and the index of the data byte must be added modulo 64K (i.e.,
// at 16 bits precision with wraparound) and the segment address only added
// afterwards.
//
// To implement fully correct segmented addressing, compute the address
// of _each byte_ with its index in `data` as follows:
//
#define IHEX_BYTE_ADDRESS(ihex, byte_index) ((((ihex)->address + (byte_index)) & 0xFFFFU) + (((ihex_address_t)((ihex)->segment)) << 4))

#else // IHEX_DISABLE_SEGMENTS:

#define IHEX_LINEAR_ADDRESS(ihex) ((ihex)->address)
#define IHEX_BYTE_ADDRESS(ihex, byte_index) ((ihex)->address + (byte_index))

#endif

// The newline string (appended to every output line, e.g., "\r\n")
#ifndef IHEX_NEWLINE_STRING
#define IHEX_NEWLINE_STRING "\r\n"
#endif

#ifdef __cplusplus
extern "C" {
#endif
	void ihex_new_buffer(struct ihex_state* ihex);

	void ihex_free_buffer(struct ihex_state* ihex);
#ifdef __cplusplus
}
#endif

// See kk_ihex_read.h and kk_ihex_write.h for function declarations!
#endif // !KK_IHEX_H
