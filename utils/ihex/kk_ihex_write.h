#ifndef KK_IHEX_WRITE_H
#define KK_IHEX_WRITE_H

#include "kk_ihex.h"
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	// Initialise the structure `ihex` for writing
	void ihex_begin_write(struct ihex_state* ihex);

	// Begin writing at the given 32-bit `address` after writing any
	// pending data at the current address.
	//
	// This can also be used to skip to a new address without calling
	// `ihex_end_write`; this allows writing sparse output.
	//
	void ihex_write_at_address(struct ihex_state* ihex, ihex_address_t address);

	// Write a single byte
	void ihex_write_byte(struct ihex_state* ihex, int b);

	// Write `count` bytes from `data`
	void ihex_write_bytes(struct ihex_state* ihex, const void* data, ihex_count_t count);

	int ihex_write_from_bin_file(struct ihex_state* ihex, const char* file);

	// End writing (flush buffers, write end of file record)
	void ihex_end_write(struct ihex_state* ihex);

	// segments are not automatically incremented when the 16-bit address
	// overflows (the default is to use 32-bit linear addressing). For segmented
	// 20-bit addressing you must manually ensure that a write does not overflow
	// the segment boundary, and call `ihex_write_at_segment` every time the
	// segment needs to be changed.
	//
#ifndef IHEX_DISABLE_SEGMENTS
	void ihex_write_at_segment(struct ihex_state* ihex, ihex_segment_t segment, ihex_address_t address);
#endif

	// Set the output line length to `length` - may be safely called only right
	// after `ihex_write_at_address` or `ihex_write_at_segment`. The maximum
	// is IHEX_LINE_MAX_LENGTH (which may be changed at compile time).
	void ihex_set_output_line_length(struct ihex_state* ihex, uint8_t line_length);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // !KK_IHEX_WRITE_H
