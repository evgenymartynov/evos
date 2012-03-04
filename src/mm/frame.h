#ifndef _MM_FRAME_H
#define _MM_FRAME_H

#include "stdint.h"

void init_frames(uint32_t total_memory);
uint32_t frame_alloc(void);
void frame_free(uint32_t frame);

#endif
