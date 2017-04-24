
/*
 * cache.h
 */


#define TRUE 1
#define FALSE 0

/* default cache parameters--can be changed */
#define WORD_SIZE 4
#define WORD_SIZE_OFFSET 2
#define DEFAULT_CACHE_SIZE (8 * 1024)
#define DEFAULT_CACHE_BLOCK_SIZE 16
#define DEFAULT_CACHE_ASSOC 1
#define DEFAULT_CACHE_WRITEBACK TRUE
#define DEFAULT_CACHE_WRITEALLOC TRUE

/* constants for settting cache parameters */
#define CACHE_PARAM_BLOCK_SIZE 0
#define CACHE_PARAM_USIZE 1
#define CACHE_PARAM_ISIZE 2
#define CACHE_PARAM_DSIZE 3
#define CACHE_PARAM_ASSOC 4
#define CACHE_PARAM_WRITEBACK 5
#define CACHE_PARAM_WRITETHROUGH 6
#define CACHE_PARAM_WRITEALLOC 7
#define CACHE_PARAM_NOWRITEALLOC 8


/* structure definitions */
typedef struct cache_line_ {
  int tag;
  int dirty;
  char* data;
} cache_line;

typedef struct cache_ {
  int size;			/* cache size */
  int associativity;		/* cache associativity */
  int n_sets;			/* number of cache sets */
  int block_size;
  cache_line** cache_line_array;
  unsigned index_mask;		/* mask to find cache index */
  int index_mask_offset;	/* number of zero bits in mask */
  
  unsigned tag_mask;		/* mask to find cache index */
  int tag_mask_offset;	/* number of zero bits in mask */
  
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
void set_cache_param();
void init_cache();
void perform_access();
void flush();
void dump_settings();
void print_stats();


// static cache* my_cache;
static cache* my_cache_data;
static cache* my_cache_instruction;

// static cache_stat* my_cache_stat;
static cache_stat* my_cache_stat_data;
static cache_stat* my_cache_stat_instruction;
/* macros */
#define LOG2(x) ((int) rint((log((double) (x))) / (log(2.0))))