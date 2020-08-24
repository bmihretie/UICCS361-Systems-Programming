extern int mm_init(void);
extern void *mm_malloc (size_t size);
extern void mm_free (void *ptr);

extern void *mm_realloc(void *ptr, size_t size);
extern void *mm_calloc (size_t nmemb, size_t size);

extern unsigned int * mm_first_hdr(void);
extern void printblock(void *bp);
extern void checkheap(int verbose);
