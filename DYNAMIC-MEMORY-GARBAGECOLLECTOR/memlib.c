/*
 * memlib.c - a module that simulates the memory system.  Needed
 * because it allows us to interleave calls from the student's malloc
 * package with the system's malloc package in libc.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

#include "memlib.h"

/* $begin memlib */
/* Private global variables */
static char *mem_heap = NULL;     /* Points to first byte of heap */
static char *mem_brk = NULL;      /* Points to last byte of heap plus 1 */
static char *mem_max_addr = NULL; /* Max legal heap addr plus 1*/

/*
 * mem_init - Initialize the memory system model
 */
#define MAX_HEAP (20*(1<<20))  /* 20 MB */
void mem_init(void)
{
  mem_heap = (char *)malloc(MAX_HEAP);
  mem_brk = (char *)mem_heap;
  mem_max_addr = (char *)(mem_heap + MAX_HEAP);
}

/*
 * mem_sbrk - Simple model of the sbrk function. Extends the heap
 *    by incr bytes and returns the start address of the new area. In
 *    this model, the heap cannot be shrunk.
 */
void *mem_sbrk(int incr)
{
  if (!mem_heap)
    mem_init();
  char *old_brk = mem_brk;

  if ( (incr < 0) || ((mem_brk + incr) > mem_max_addr)) {
    errno = ENOMEM;
    fprintf(stderr, "ERROR: mem_sbrk failed. Ran out of memory...\n");
    return (void *)-1;
  }
  mem_brk += incr;
  return (void *)old_brk;
}

/*
 * mem_heapsize() - returns the heap size in bytes
 */
size_t mem_heapsize()
{
  return (size_t)((void *)mem_brk - (void *)mem_heap);
}
