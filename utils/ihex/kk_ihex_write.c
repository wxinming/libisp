#include "kk_ihex_write.h"

#define IHEX_START ':'

// Default number of data bytes written per line
#if IHEX_LINE_MAX_LENGTH >= 32
#define IHEX_DEFAULT_OUTPUT_LINE_LENGTH 32
#else
#define IHEX_DEFAULT_OUTPUT_LINE_LENGTH IHEX_LINE_MAX_LENGTH
#endif

#ifndef IHEX_MAX_OUTPUT_LINE_LENGTH
#define IHEX_MAX_OUTPUT_LINE_LENGTH IHEX_LINE_MAX_LENGTH
#endif

// Length of the write buffer required
#define IHEX_WRITE_BUFFER_LENGTH (1+2+4+2+(IHEX_MAX_OUTPUT_LINE_LENGTH*2)+2+sizeof(IHEX_NEWLINE_STRING))
#define ADDRESS_HIGH_MASK ((ihex_address_t) 0xFFFF0000U)
#define ADDRESS_HIGH_BYTES(addr) ((addr) >> 16)

#define HEX_DIGIT(n) ((char)((n) + (((n) < 10) ? '0' : ('A' - 10))))

#if IHEX_MAX_OUTPUT_LINE_LENGTH > IHEX_LINE_MAX_LENGTH
#error "IHEX_MAX_OUTPUT_LINE_LENGTH > IHEX_LINE_MAX_LENGTH"
#endif

void ihex_begin_write(struct ihex_state* ihex) {
	ihex->address = 0;
#ifndef IHEX_DISABLE_SEGMENTS
	ihex->segment = 0;
#endif
	ihex->flags = 0;
	ihex->line_length = IHEX_DEFAULT_OUTPUT_LINE_LENGTH;
	ihex->length = 0;
	ihex->line_number = 1;
}

static char* ihex_buffer_byte(char* w, const uint8_t byte) {
	uint8_t n = (byte & 0xF0U) >> 4; // high nybble
	*w++ = HEX_DIGIT(n);
	n = byte & 0x0FU; // low nybble
	*w++ = HEX_DIGIT(n);
	return w;
}

static char* ihex_buffer_word(char* w, const uint_fast16_t word,
	uint8_t* const  checksum) {
	uint8_t byte = (word >> 8) & 0xFFU; // high byte
	w = ihex_buffer_byte(w, (uint8_t)byte);
	*checksum += byte;
	byte = word & 0xFFU; // low byte
	*checksum += byte;
	return ihex_buffer_byte(w, (uint8_t)byte);
}

static char* ihex_buffer_newline(char* w) {
	const char* r = IHEX_NEWLINE_STRING;
	do {
		*w++ = *r++;
	} while (*r);
	return w;
}

static void ihex_copy_buffer(struct ihex_state* ihex, const char* ihex_write_buffer) {
	size_t ihex_write_length = strlen(ihex_write_buffer);
	if (ihex->buffer.heap_size < ihex->buffer.size + ihex_write_length) {
		ihex->buffer.heap_size *= 2;
		ihex->buffer.data = (uint8_t*)realloc(ihex->buffer.data, ihex->buffer.heap_size);
	}
	memcpy(ihex->buffer.data + ihex->buffer.size, ihex_write_buffer, ihex_write_length);
	ihex->buffer.size += ihex_write_length;
}

static void ihex_write_end_of_file(struct ihex_state* ihex) {
	char ihex_write_buffer[IHEX_WRITE_BUFFER_LENGTH] = { 0 };
	char* w = ihex_write_buffer;
	*w++ = IHEX_START; // :
#if 1
	* w++ = '0'; *w++ = '0'; // length
	*w++ = '0'; *w++ = '0'; *w++ = '0'; *w++ = '0'; // address
	*w++ = '0'; *w++ = '1'; // record type
	*w++ = 'F'; *w++ = 'F'; // checksum
#else
	w = ihex_buffer_byte(w, 0); // length
	w = ihex_buffer_byte(w, 0); // address msb
	w = ihex_buffer_byte(w, 0); // address lsb
	w = ihex_buffer_byte(w, IHEX_END_OF_FILE_RECORD); // record type
	w = ihex_buffer_byte(w, (uint8_t)~IHEX_END_OF_FILE_RECORD + 1U); // checksum
#endif
	w = ihex_buffer_newline(w);
	*w = '\0';
	ihex_copy_buffer(ihex, ihex_write_buffer);
}

static void ihex_write_extended_address(struct ihex_state* ihex,
	const ihex_segment_t address,
	const uint8_t type) {
	char ihex_write_buffer[IHEX_WRITE_BUFFER_LENGTH] = { 0 };
	char* w = ihex_write_buffer;
	uint8_t sum = type + 2U;

	*w++ = IHEX_START;              // :
	w = ihex_buffer_byte(w, 2U);    // length
	w = ihex_buffer_byte(w, 0);     // 16-bit address msb
	w = ihex_buffer_byte(w, 0);     // 16-bit address lsb
	w = ihex_buffer_byte(w, type);  // record type
	w = ihex_buffer_word(w, address, &sum); // high bytes of address
	w = ihex_buffer_byte(w, (uint8_t)~sum + 1U); // checksum
	w = ihex_buffer_newline(w);
	*w = '\0';
	ihex_copy_buffer(ihex, ihex_write_buffer);
}

// Write out `ihex->data`
//
static void ihex_write_data(struct ihex_state* ihex) {
	uint_fast8_t len = ihex->length;
	uint8_t sum = len;
	char ihex_write_buffer[IHEX_WRITE_BUFFER_LENGTH] = { 0 };
	char* w = ihex_write_buffer;

	if (!len) {
		return;
	}

	if (ihex->flags & IHEX_FLAG_ADDRESS_OVERFLOW) {
		ihex_write_extended_address(ihex, ADDRESS_HIGH_BYTES(ihex->address),
			IHEX_EXTENDED_LINEAR_ADDRESS_RECORD);
		ihex->flags &= ~IHEX_FLAG_ADDRESS_OVERFLOW;
	}

	// :
	*w++ = IHEX_START;

	// length
	w = ihex_buffer_byte(w, len);
	ihex->length = 0;

	// 16-bit address
	{
		uint_fast16_t addr = ihex->address & 0xFFFFU;
		ihex->address += len;
		if ((0xFFFFU - addr) < len) {
			// signal address overflow (need to write extended address)
			ihex->flags |= IHEX_FLAG_ADDRESS_OVERFLOW;
		}
		w = ihex_buffer_word(w, addr, &sum);
	}

	// record type
	w = ihex_buffer_byte(w, IHEX_DATA_RECORD);
	//sum += IHEX_DATA_RECORD; // IHEX_DATA_RECORD is zero, so NOP

	// data
	{
		uint8_t* r = ihex->data;
		do {
			uint8_t byte = *r++;
			sum += byte;
			w = ihex_buffer_byte(w, byte);
		} while (--len);
	}

	// checksum
	w = ihex_buffer_byte(w, ~sum + 1U);

	w = ihex_buffer_newline(w);
	*w = '\0';
	ihex_copy_buffer(ihex, ihex_write_buffer);
}

void ihex_write_at_address(struct ihex_state* ihex, ihex_address_t address) {
	if (ihex->length) {
		// flush any existing data
		ihex_write_data(ihex);
	}

	const ihex_address_t page = address & ADDRESS_HIGH_MASK;
	if ((ihex->address & ADDRESS_HIGH_MASK) != page) {
		// write a new extended address if needed
		ihex->flags |= IHEX_FLAG_ADDRESS_OVERFLOW;
	}
	else if (ihex->address != page) {
		ihex->flags &= ~IHEX_FLAG_ADDRESS_OVERFLOW;
	}

	ihex->address = address;
	ihex_set_output_line_length(ihex, ihex->line_length);
}

void ihex_set_output_line_length(struct ihex_state* ihex, uint8_t line_length) {
#if IHEX_MAX_OUTPUT_LINE_LENGTH < 255
	if (line_length > IHEX_MAX_OUTPUT_LINE_LENGTH) {
		line_length = IHEX_MAX_OUTPUT_LINE_LENGTH;
	}
	else
#endif
		if (!line_length) {
			line_length = IHEX_DEFAULT_OUTPUT_LINE_LENGTH;
		}
	ihex->line_length = line_length;
}

#ifndef IHEX_DISABLE_SEGMENTS
void ihex_write_at_segment(struct ihex_state* ihex,
	ihex_segment_t segment,
	ihex_address_t address) {
	ihex_write_at_address(ihex, address);
	if (ihex->segment != segment) {
		// clear segment
		ihex_write_extended_address(ihex, (ihex->segment = segment),
			IHEX_EXTENDED_SEGMENT_ADDRESS_RECORD);
	}
}
#endif

void ihex_write_byte(struct ihex_state* ihex, const int byte) {
	if (ihex->line_length <= ihex->length) {
		ihex_write_data(ihex);
	}
	ihex->data[(ihex->length)++] = (uint8_t)byte;
}

void ihex_write_bytes(struct ihex_state* ihex,
	const void* buf,
	ihex_count_t count) {
	const uint8_t* r = (uint8_t*)buf;
	while (count > 0) {
		if (ihex->line_length > ihex->length) {
			uint_fast8_t i = ihex->line_length - ihex->length;
			uint8_t* w = ihex->data + ihex->length;
			i = ((ihex_count_t)i > count) ? (uint_fast8_t)count : i;
			count -= i;
			ihex->length += i;
			do {
				*w++ = *r++;
			} while (--i);
		}
		else {
			ihex_write_data(ihex);
		}
	}
}

int ihex_write_from_bin_file(struct ihex_state* ihex, const char* file)
{
	int ret = 0;
	FILE* in_file = NULL;
	do
	{
		ret = fopen_s(&in_file, file, "rb");
		if (ret != 0) {
			break;
		}

		char in_buf[256] = { 0 };
		while (!feof(in_file)) {
			size_t count = fread(in_buf, 1, sizeof(in_buf), in_file);
			ihex_write_bytes(ihex, in_buf, count);
		}

	} while (0);
	if (in_file) {
		fclose(in_file);
	}
	return ret;
}

void ihex_end_write(struct ihex_state* ihex) {
	ihex_write_data(ihex); // flush any remaining data
	ihex_write_end_of_file(ihex);
}

