#include <unistd.h>

void mem_init(void);
void *mem_sbrk(int incr);
size_t mem_heapsize(void);
