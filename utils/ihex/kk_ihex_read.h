#ifndef KK_IHEX_READ_H
#define KK_IHEX_READ_H

#include "kk_ihex.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	// Begin reading at address 0
	void ihex_begin_read(struct ihex_state* ihex);

	// Begin reading at `address` (the lowest 16 bits of which will be ignored);
	// this is required only if the high bytes of the 32-bit starting address
	// are not specified in the input data and they are non-zero
	void ihex_read_at_address(struct ihex_state* ihex, ihex_address_t address);

	// Read a single character
	// return 0 successed, -1 checksum error, -2 length error
	int ihex_read_byte(struct ihex_state* ihex, char chr);

	// Read `count` bytes from `data`
	// return 0 successed, -1 checksum error, -2 length error
	int ihex_read_bytes(struct ihex_state* ihex, const char* data, ihex_count_t count);

	// return 0 successed, -1 checksum error, -2 length error, >0 open file error
	int ihex_read_from_hex_file(struct ihex_state* ihex, const char* file);

	// End reading (may call `ihex_data_read` if there is data waiting)
	void ihex_end_read(struct ihex_state* ihex);

	// Begin reading at `segment`; this is required only if the initial segment
	// is not specified in the input data and it is non-zero.
	//
#ifndef IHEX_DISABLE_SEGMENTS
	void ihex_read_at_segment(struct ihex_state* ihex, ihex_segment_t segment);
#endif

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !KK_IHEX_READ_H
