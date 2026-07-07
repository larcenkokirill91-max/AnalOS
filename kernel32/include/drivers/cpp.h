#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void heap_init(void);
void* malloc(unsigned int size);
void free(void* ptr);

#ifdef __cplusplus
}
#endif
