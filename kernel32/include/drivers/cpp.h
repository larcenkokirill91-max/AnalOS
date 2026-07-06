#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Сообщаем C++ компилятору, что функции malloc и heap_init 
// реализованы в Си-файлах ядра и их можно использовать
void heap_init(void);
void* malloc(unsigned int size);
void free(void* ptr);

#ifdef __cplusplus
}
#endif
