#include "kk_ihex.h"

void ihex_new_buffer(struct ihex_state* ihex)
{
	ihex->buffer.data = (uint8_t*)malloc(IHEX_BUFFER_SIZE);
	ihex->buffer.size = 0;
	ihex->buffer.heap_size = IHEX_BUFFER_SIZE;
}

void ihex_free_buffer(struct ihex_state* ihex)
{
	if (ihex->buffer.data) {
		free(ihex->buffer.data);
		ihex->buffer.data = NULL;
	}
	ihex->buffer.heap_size = 0;
	ihex->buffer.size = 0;
}
