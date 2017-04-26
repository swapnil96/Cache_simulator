
/*
 * cache.h
 */
#ifndef CACHE_H
#define CACHE_H

#define TRUE 1
#define FALSE 0

#define DATA 0
#define INSTRUCTION 1

/* default cache parameters--can be changed */
#define WORD_SIZE 4
#define WORD_SIZE_OFFSET 2
#define DEFAULT_CACHE_SIZE (8 * 1024)
#define DEFAULT_CACHE_BLOCK_SIZE 16
#define DEFAULT_CACHE_ASSOC 1
#define DEFAULT_CACHE_WRITEBACK TRUE
#define DEFAULT_CACHE_WRITEALLOC TRUE


/* constants for settting cache parameters */
#define CACHE_PARAM_SIZE_I 0
#define CACHE_PARAM_SIZE_D 1
#define CACHE_PARAM_ASSOC_I 2
#define CACHE_PARAM_ASSOC_D 3
#define CACHE_PARAM_BLOCK_SIZE_I 4
#define CACHE_PARAM_BLOCK_SIZE_D 5
#define CACHE_PARAM_WRITETHROUGH_I 6
#define CACHE_PARAM_WRITETHROUGH_D 7


/* structure definitions */
typedef struct cache_line_ {
  int tag;
  int dirty;
  char* data;
  int used;
} cache_line;

typedef struct cache_ {
  int size;			/* cache size */
  int associativity;		/* cache associativity */
  int n_sets;			/* number of cache sets */
  int lines;      /*Total number of lines in the cache*/
  int block_size;
  cache_line** cache_line_array;
  
  unsigned set_mask;		/* mask to find cache set */
  int set_mask_offset;	/* number of zero bits in mask */
  
  unsigned tag_mask;		/* mask to find cache index */
  int tag_mask_offset;	/* number of zero bits in mask */
  
  unsigned byte_mask;		/* mask to find cache index */
  int byte_mask_offset;	/* number of zero bits in mask */
  
  unsigned word_mask;		/* mask to find cache index */
  int word_mask_offset;	/* number of zero bits in mask */
  
  // int *set_contents;		/* number of valid entries in set */
  // int contents;			/* number of valid entries in cache */
} cache;

typedef struct cache_stat_ {
  int accesses;			/* number of memory references */
  int misses;			/* number of cache misses */
  int replacements;		/* number of misses that cause replacments */
  int demand_fetches;		/* number of fetches in words from memory */
  int copies_back;		/* number of write backs in words to memory */
} cache_stat;


/* function prototypes */
void set_cache_param(int param, int value);
void init_cache();
char* perform_access(unsigned addr, unsigned access_type, unsigned type, char* store);
void flush();
void dump_settings();
void print_settings();
void print_stats();


// static cache* my_cache;
static cache* my_cache_d;
static cache* my_cache_i;

// static cache_stat* my_cache_stat;
static cache_stat* my_cache_stat_data;
static cache_stat* my_cache_stat_instruction;
/* macros */
#define LOG2(x) ((int) rint((log((double) (x))) / (log(2.0))))

#endif