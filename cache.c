
// To_do

// Breakpoint 
// Delete Breakpoint
// Continue to next breakpoint 
// Run to the end

// Instructions 

// Jump 
// Data move

// Memory length = 2 ^ 26 (d = 26) : for block = 4 :- 4 bytes in a block to a total of 64 MB - Number of blocks = 2 ^ 26 / 4


/*
 * cache.c
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "cache.h"
#include "main.h"

/* cache configuration parameters */
static int cache_split = 0;
static int cache_usize = DEFAULT_CACHE_SIZE;
static int cache_isize = DEFAULT_CACHE_SIZE; 
static int cache_dsize = DEFAULT_CACHE_SIZE;
static int cache_block_size = DEFAULT_CACHE_BLOCK_SIZE;
static int words_per_block = DEFAULT_CACHE_BLOCK_SIZE / WORD_SIZE;
static int cache_assoc = DEFAULT_CACHE_ASSOC;
static int cache_writeback = DEFAULT_CACHE_WRITEBACK;
static int cache_writealloc = DEFAULT_CACHE_WRITEALLOC;

/* cache model data structures */
// static Pcache icache;
// static Pcache dcache;
// static cache c1;
static cache* my_cache;
static cache_stat* my_cache_stat;
// static cache_stat cache_stat_inst;
// static cache_stat cache_stat_data;

/************************************************************/
void set_cache_param(int param, int value)
{
	switch (param) 
	{
  		case CACHE_PARAM_BLOCK_SIZE:
    		cache_block_size = value;
    		words_per_block = value / WORD_SIZE;
    	break;
  		case CACHE_PARAM_USIZE:
    		cache_split = FALSE;
    		cache_usize = value;
    	break;
  		case CACHE_PARAM_ISIZE:
    		cache_split = TRUE;
    		cache_isize = value;
    	break;
  		case CACHE_PARAM_DSIZE:
    		cache_split = TRUE;
    		cache_dsize = value;
    	break;
  		case CACHE_PARAM_ASSOC:
    		cache_assoc = value;
    	break;
  		case CACHE_PARAM_WRITEBACK:
    		cache_writeback = TRUE;
    	break;
  		case CACHE_PARAM_WRITETHROUGH:
    		cache_writeback = FALSE;
    	break;
  		case CACHE_PARAM_WRITEALLOC:
    		cache_writealloc = TRUE;
    	break;
  		case CACHE_PARAM_NOWRITEALLOC:
    		cache_writealloc = FALSE;
    	break;
  default:
    printf("error set_cache_param: bad parameter value\n");
    exit(-1);
  }

}
/************************************************************/

/************************************************************/
// To_do - called once to buildand initialize the cache model data structures

void print_settings()
{
	printf("*** SET CACHE SETTINGS ***\n");

  	printf("  Associativity: \t%d\n", my_cache->associativity);
  	printf("  Block size: \t%d\n", my_cache->block_size);
  	printf("  N_Sets: \t%d\n", my_cache->n_sets);
  	printf("  Tag_mask: \t%d\n", my_cache->tag_mask);
  	printf("  Tag_mask_offset: \t%d\n", my_cache->tag_mask_offset);
	printf("  Index_mask: \t%d\n", my_cache->index_mask);
  	printf("  Index_mask_offset: \t%d\n", my_cache->index_mask_offset);
}


double bin_to_dec(char* bin)
{
    double ans = 0;
    for(int i = 31; i >= 0; i--)
        ans += (bin[i] - '0') * pow(2,31-i);

    ans = bin[0] == '0' ? ans : -1*((double)pow(2, 32) - ans);
    return ans;
}


void init_cache()
{

	/* initialize the cache.*/
	my_cache = (cache*)malloc(sizeof(cache));

	my_cache->n_sets = cache_usize / cache_block_size;

	my_cache->size = cache_usize;

	my_cache->associativity = cache_assoc;

	my_cache->cache_line_array = (cache_line**)malloc(my_cache->n_sets * sizeof(cache_line*));

	my_cache->associativity = cache_assoc;

	my_cache->block_size = cache_block_size;

	for (int i = 0; i < my_cache->n_sets; i++)
	{
		my_cache->cache_line_array[i] = (cache_line*)malloc(sizeof(cache_line));
		my_cache->cache_line_array[i]->dirty = 0;
		my_cache->cache_line_array[i]->tag = 0;
	}

	// Calculating index_mask and offset
	int offset_bits = LOG2(my_cache->block_size);
	int s_bits = LOG2(my_cache->size);

	char* temp;
	temp = strdup("00000000000000000000000000000000");
	for(int i = 32 - s_bits; i < 32 - offset_bits; i++)
		temp[i] = '1';
	
	my_cache->index_mask = (int)bin_to_dec(temp);
	my_cache->index_mask_offset = offset_bits;

	// Calculating tag_mask and offset
	temp = strdup("00000000000000000000000000000000");
	for(int i = 0; i < 32 - s_bits; i++)
		temp[i] = '1';
	
	my_cache->tag_mask = (int)bin_to_dec(temp);
	my_cache->tag_mask_offset = 32 - s_bits;
	print_settings();


	/*Cache statistics data structures */	
	my_cache_stat = (cache_stat*)malloc(sizeof(cache_stat));
	my_cache_stat->accesses = 0;
	my_cache_stat->misses = 0;
	my_cache_stat->replacements = 0;
	my_cache_stat->demand_fetches = 0;
	my_cache_stat->copies_back = 0;
}
/************************************************************/

/************************************************************/

// To_do - performaccess()is called once for each iterationof the simulator loop to simulate a single memory reference to the cache
void perform_access(unsigned addr, unsigned access_type)
{
	/* handle an access to the cache */

	int offset_bits = LOG2(my_cache->block_size);
	int s_bits = LOG2(my_cache->size);

	int index_bits = s_bits - offset_bits; 



	int block_number = addr >> LOG2(my_cache->block_size);



	int memory_number = block_number * my_cache->block_size;

	int index = (addr & my_cache->index_mask) >> my_cache->index_mask_offset;

	int tag = (addr & my_cache->tag_mask) >> my_cache->tag_mask_offset;

	cache_line* temp = my_cache->cache_line_array[index];

	if (tag == temp->tag)
	{
		switch(access_type)
		{
			case 0:


				break;
			case 1:

				break;
			case 2:

				break;
		}		
	}


	// if tag == 



}
/************************************************************/

/************************************************************/
void flush()
{

  /* flush the cache */

}
/************************************************************/

/************************************************************/
void dump_settings()
{
	printf("--------------_Cache parameters------------------------------\n");
	printf("\tCache Block size -> %d\n", cache_block_size);
	printf("\tCache d size -> %d\n", cache_dsize);
	printf("\tCache i size -> %d\n", cache_isize);
	printf("\tCache u size -> %d\n", cache_usize);
	printf("\tCache associativity -> %d\n", cache_assoc);
	printf("\tCache words_per_block -> %d\n", cache_block_size/ 4);
	printf("\tCache split -> %d\n", cache_split);
	printf("\tCache writeback -> %d\n", cache_writeback);
	printf("\tCache writealloc -> %d\n", cache_writealloc);
  	
	printf("*** CACHE SETTINGS ***\n");
  	if (cache_split) 
	{
    	("  Split I- D-cache\n");
    	("  I-cache size: \t%d\n", cache_isize);
    	("  D-cache size: \t%d\n", cache_dsize);
  	} 
	else 
	{
		printf("  Unified I- D-cache\n");
		printf("  Size: \t%d\n", cache_usize);
  	}
  	printf("  Associativity: \t%d\n", cache_assoc);
  	printf("  Block size: \t%d\n", cache_block_size);
  	printf("  Write policy: \t%s\n", cache_writeback ? "WRITE BACK" : "WRITE THROUGH");
  	printf("  Allocation policy: \t%s\n", cache_writealloc ? "WRITE ALLOCATE" : "WRITE NO ALLOCATE");
}
/************************************************************/

/************************************************************/
void print_stats()
{
  printf("\n*** CACHE STATISTICS ***\n");

  printf(" INSTRUCTIONS\n");
  printf("  accesses:  %d\n", cache_stat_inst.accesses);
  printf("  misses:    %d\n", cache_stat_inst.misses);
  if (!cache_stat_inst.accesses)
    printf("  miss rate: 0 (0)\n"); 
  else
    printf("  miss rate: %2.4f (hit rate %2.4f)\n", 
	 (float)cache_stat_inst.misses / (float)cache_stat_inst.accesses,
	 1.0 - (float)cache_stat_inst.misses / (float)cache_stat_inst.accesses);
  printf("  replace:   %d\n", cache_stat_inst.replacements);

  printf(" DATA\n");
  printf("  accesses:  %d\n", cache_stat_data.accesses);
  printf("  misses:    %d\n", cache_stat_data.misses);
  if (!cache_stat_data.accesses)
    printf("  miss rate: 0 (0)\n"); 
  else
    printf("  miss rate: %2.4f (hit rate %2.4f)\n", 
	 (float)cache_stat_data.misses / (float)cache_stat_data.accesses,
	 1.0 - (float)cache_stat_data.misses / (float)cache_stat_data.accesses);
  printf("  replace:   %d\n", cache_stat_data.replacements);

  printf(" TRAFFIC (in words)\n");
  printf("  demand fetch:  %d\n", cache_stat_inst.demand_fetches + 
	 cache_stat_data.demand_fetches);
  printf("  copies back:   %d\n", cache_stat_inst.copies_back +
	 cache_stat_data.copies_back);
}
/************************************************************/