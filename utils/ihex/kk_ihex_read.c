#include "kk_ihex_read.h"

#define IHEX_START ':'

#define ADDRESS_HIGH_MASK ((ihex_address_t) 0xFFFF0000U)

enum ihex_read_state {
	READ_WAIT_FOR_START = 0,
	READ_COUNT_HIGH = 1,
	READ_COUNT_LOW,
	READ_ADDRESS_MSB_HIGH,
	READ_ADDRESS_MSB_LOW,
	READ_ADDRESS_LSB_HIGH,
	READ_ADDRESS_LSB_LOW,
	READ_RECORD_TYPE_HIGH,
	READ_RECORD_TYPE_LOW,
	READ_DATA_HIGH,
	READ_DATA_LOW
};

#define IHEX_READ_RECORD_TYPE_MASK 0x07
#define IHEX_READ_STATE_MASK 0x78
#define IHEX_READ_STATE_OFFSET 3


void ihex_begin_read(struct ihex_state* ihex) {
	ihex->address = 0;
#ifndef IHEX_DISABLE_SEGMENTS
	ihex->segment = 0;
#endif
	ihex->flags = 0;
	ihex->type = 0;
	ihex->line_length = 0;
	ihex->length = 0;
	ihex->line_number = 1;
}

void ihex_read_at_address(struct ihex_state* const ihex, ihex_address_t address) {
	ihex_begin_read(ihex);
	ihex->address = address;
}

#ifndef IHEX_DISABLE_SEGMENTS
void ihex_read_at_segment(struct ihex_state* const ihex, ihex_segment_t segment) {
	ihex_begin_read(ihex);
	ihex->segment = segment;
}
#endif

void ihex_end_read(struct ihex_state* ihex)
{
	uint_fast8_t type = ihex->flags & IHEX_READ_RECORD_TYPE_MASK;
	uint_fast8_t sum;
	if ((sum = ihex->length) == 0 && type == IHEX_DATA_RECORD) {
		return;
	}

	// compute and validate checksum
	const uint8_t* const eptr = ihex->data + sum;
	const uint8_t* r = ihex->data;
	sum += type + (ihex->address & 0xFFU) + ((ihex->address >> 8) & 0xFFU);
	while (r != eptr) {
		sum += *r++;
	}
	sum = (~sum + 1U) ^ *eptr; // *eptr is the received checksum

	if (type == IHEX_EXTENDED_LINEAR_ADDRESS_RECORD) {
		ihex->address &= 0xFFFFU;
		ihex->address |= (((ihex_address_t)ihex->data[0]) << 24) |
			(((ihex_address_t)ihex->data[1]) << 16);
#ifndef IHEX_DISABLE_SEGMENTS
	}
	else if (type == IHEX_EXTENDED_SEGMENT_ADDRESS_RECORD) {
		ihex->segment = (ihex_segment_t)((ihex->data[0] << 8) | ihex->data[1]);
#endif
	}
	ihex->length = 0;
	ihex->flags = 0;
}

static int ihex_end_read_internal(struct ihex_state* ihex) {
	uint_fast8_t type = ihex->flags & IHEX_READ_RECORD_TYPE_MASK;
	ihex->type = type;
	uint_fast8_t sum;
	if ((sum = ihex->length) == 0 && type == IHEX_DATA_RECORD) {
		return 0;
	}

	// compute and validate checksum
	const uint8_t* const eptr = ihex->data + sum;
	const uint8_t* r = ihex->data;
	sum += type + (ihex->address & 0xFFU) + ((ihex->address >> 8) & 0xFFU);
	while (r != eptr) {
		sum += *r++;
	}
	sum = (~sum + 1U) ^ *eptr; // *eptr is the received checksum

	if (sum != 0) {
		//校验错误
		return -1;
	}
	else if (ihex->length < ihex->line_length) {
		//长度错误
		return -2;
	}

	if (type == IHEX_DATA_RECORD) {
		if (ihex->buffer.heap_size < ihex->buffer.size + ihex->length)
		{
			ihex->buffer.heap_size *= 2;
			ihex->buffer.data = (uint8_t*)realloc(ihex->buffer.data, ihex->buffer.heap_size);
		}
		memcpy(ihex->buffer.data + ihex->buffer.size, ihex->data, ihex->length);
		ihex->buffer.size += ihex->length;
	}
	else if (type == IHEX_END_OF_FILE_RECORD) {
	}
	else if (type == IHEX_START_SEGMENT_ADDRESS_RECORD) {
	}
	else if (type == IHEX_START_LINEAR_ADDRESS_RECORD) {
	}
	else if (type == IHEX_EXTENDED_LINEAR_ADDRESS_RECORD) {
		ihex->address &= 0xFFFFU;
		ihex->address |= (((ihex_address_t)ihex->data[0]) << 24) | (((ihex_address_t)ihex->data[1]) << 16);
#ifndef IHEX_DISABLE_SEGMENTS
	}
	else if (type == IHEX_EXTENDED_SEGMENT_ADDRESS_RECORD) {
		ihex->segment = (ihex_segment_t)((ihex->data[0] << 8) | ihex->data[1]);
#endif
	}
	++ihex->line_number;
	ihex->length = 0;
	ihex->flags = 0;

	return 0;
}

int ihex_read_byte(struct ihex_state* ihex, const char byte) {
	int ret = 0;
	uint_fast8_t b = (uint_fast8_t)byte;
	uint_fast8_t len = ihex->length;
	uint_fast8_t state = (ihex->flags & IHEX_READ_STATE_MASK);
	ihex->flags ^= state; // turn off the old state
	state >>= IHEX_READ_STATE_OFFSET;

	if (b >= '0' && b <= '9') {
		b -= '0';
	}
	else if (b >= 'A' && b <= 'F') {
		b -= 'A' - 10;
	}
	else if (b >= 'a' && b <= 'f') {
		b -= 'a' - 10;
	}
	else if (b == IHEX_START) {
		// sync to a new record at any state
		state = READ_COUNT_HIGH;
		goto end_read;
	}
	else {
		// ignore unknown characters (e.g., extra whitespace)
		goto save_read_state;
	}

	if (!(++state & 1)) {
		// high nybble, store temporarily at end of data:
		b <<= 4;
		ihex->data[len] = b;
	}
	else {
		// low nybble, combine with stored high nybble:
		b = (ihex->data[len] |= b);
		// We already know the lowest bit of `state`, dropping it may produce
		// smaller code, hence the `>> 1` in switch and its cases.
		switch (state >> 1) {
		default:
			// remain in initial state while waiting for :
			return ret;
		case (READ_COUNT_LOW >> 1):
			// data length
			ihex->line_length = b;
#if IHEX_LINE_MAX_LENGTH < 255
			if (b > IHEX_LINE_MAX_LENGTH) {
				ihex_end_read_internal(ihex);
				return;
			}
#endif
			break;
		case (READ_ADDRESS_MSB_LOW >> 1):
			// high byte of 16-bit address
			ihex->address &= ADDRESS_HIGH_MASK; // clear the 16-bit address
			ihex->address |= ((ihex_address_t)b) << 8U;
			break;
		case (READ_ADDRESS_LSB_LOW >> 1):
			// low byte of 16-bit address
			ihex->address |= (ihex_address_t)b;
			break;
		case (READ_RECORD_TYPE_LOW >> 1):
			// record type
			if (b & ~IHEX_READ_RECORD_TYPE_MASK) {
				// skip unknown record types silently
				return ret;
			}
			ihex->flags = (ihex->flags & ~IHEX_READ_RECORD_TYPE_MASK) | b;
			break;
		case (READ_DATA_LOW >> 1):
			if (len < ihex->line_length) {
				// data byte
				ihex->length = len + 1;
				state = READ_DATA_HIGH;
				goto save_read_state;
			}
			// end of line (last "data" byte is checksum)
			state = READ_WAIT_FOR_START;
		end_read:
			ret = ihex_end_read_internal(ihex);
		}
	}
save_read_state:
	ihex->flags |= state << IHEX_READ_STATE_OFFSET;
	return ret;
}

int ihex_read_bytes(struct ihex_state* ihex, const char* data, ihex_count_t count) {
	int ret = 0;
	while (count > 0) {
		if ((ret = ihex_read_byte(ihex, *data++)) != 0) {
			break;
		}
		--count;
	}
	return ret;
}

int ihex_read_from_hex_file(struct ihex_state* ihex, const char* file)
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
		while (fgets(in_buf, sizeof(in_buf), in_file)) {
			if ((ret = ihex_read_bytes(ihex, in_buf, sizeof(in_buf)) != 0)) {
				break;
			}
		}
	} while (0);
	if (in_file) {
		fclose(in_file);
	}
	return ret;
}

