#include "hw4.h"
#include "mm.h"
#include "memlib.h"
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#define NR ((1000))

int main(int argc, char **argv)
{
  memset(root_table, 0, sizeof(root_table[0]) * ROOT_NR);
  heap_stat("before first round of allocations");

  // the most basic allocation and clearing pointer exercise.
  // This only checks for following the root set pointers one level.
  for (int i = 0; i < ROOT_NR; i++) {
    root_table[i] = mm_malloc(sizeof(struct node) + i);
    root_table[i]->next = NULL;
    //printf("[%3d] %p\n", i, root_table[i]);
  }
  heap_stat("after first round of allocations");
  gc();
  heap_stat("after first gc everything should remain the same");

  // remove some references
  for (int i = 0; i < ROOT_NR; i += 2)
    root_table[i] = 0;
  gc();
  heap_stat("after removing half of the nodes");

  // remove all remaining references
  for (int i = 1; i < ROOT_NR; i += 2)
    root_table[i] = 0;
  gc();
  heap_stat("after removing everything");

  // create a long chain of nodes at [0]
  for (int i = 0; i < NR; i++) {
    struct node * node = mm_malloc(sizeof (struct node) + i);
    node->next = root_table[0];
    root_table[0] = node;
  }
  heap_stat("created one long chain of nodes");
  gc();
  heap_stat("after gc everything should remain the same");
  root_table[0] = NULL;
  gc();
  heap_stat("removed the root, everything should be freed");

  // just leak
  // mm_malloc(0) returns NULL
  for (int i = 0; i < NR; i++)
    (void)mm_malloc(i);
  heap_stat("allocate and leak immediately");
  gc();
  heap_stat("everything should be freed");

  // allocate and insert or leak by random
  // a very common source of leaks in complex data structures
  // after explicit free() AND gc() it should be completely clean
  for (int i = 0; i < NR; i++) {
    struct node * node = mm_malloc(sizeof(struct node) + i);
    int idx = random() % ROOT_NR;
    int leak = random() % 3; // 0, 1, 2
    switch(leak) {
    case 0: // don't leak
      node->next = root_table[idx];
      root_table[idx] = node;
      break;

    case 1: // others
      node->next = NULL;
      root_table[idx] = node;
      break;

    case 2: // itself
      break;
    default:
      break;
    }
  }
  heap_stat("before free and GC");
  // now free all known nodes
  for (int i = 0; i < ROOT_NR; i++) {
    while (root_table[i]) {
      struct node * next = root_table[i]->next;
      mm_free(root_table[i]);
      root_table[i] = next;
    }
  }
  //heap_stat("this may vary"); // don't check this
  gc();
  heap_stat("everything should have gone");
  return 0;
}
