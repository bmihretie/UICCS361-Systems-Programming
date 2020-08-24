#include <unistd.h>

struct node {
  struct node * next;
  int key; // user data; ignored
}; 

#define ROOT_NR ((1000))
extern struct node * root_table[ROOT_NR];

extern void gc();
extern void heap_stat(const char * msg);
