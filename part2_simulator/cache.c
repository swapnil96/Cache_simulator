
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
// static cache* my_cache;

static cache* my_cache_data;
static cache* my_cache_instruction;

static cache_stat* my_cache_stat_data;
static cache_stat* my_cache_stat_instruction;
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

  	printf("  Associativity: \t%d\n", my_cache_data->associativity);
  	printf("  Block size: \t%d\n", my_cache_data->block_size);
  	printf("  N_Sets: \t%d\n", my_cache_data->n_sets);
  	printf("  Tag_mask: \t%d\n", my_cache_data->tag_mask);
  	printf("  Tag_mask_offset: \t%d\n", my_cache_data->tag_mask_offset);
	printf("  Index_mask: \t%d\n", my_cache_data->index_mask);
  	printf("  Index_mask_offset: \t%d\n", my_cache_data->index_mask_offset);
  	printf("  Split: \t%d\n", cache_split);
	  
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
	int offset_bits;
	int s_bits;
	char* temp;

	switch(cache_split)
	{
		case TRUE:
			my_cache_data = (cache*)malloc(sizeof(cache));
			my_cache_data->n_sets = cache_dsize / cache_block_size;
			my_cache_data->size = cache_dsize;
			my_cache_data->associativity = cache_assoc;
			my_cache_data->cache_line_array = (cache_line**)malloc(my_cache_data->n_sets * sizeof(cache_line*));
			my_cache_data->block_size = cache_block_size;
			for (int i = 0; i < my_cache_data->n_sets; i++)
			{
				my_cache_data->cache_line_array[i] = (cache_line*)malloc(sizeof(cache_line));
				my_cache_data->cache_line_array[i]->dirty = -1;
				my_cache_data->cache_line_array[i]->tag = -1;
			}

			// Calculating index_mask and offset
			offset_bits = LOG2(my_cache_data->block_size);
			s_bits = LOG2(my_cache_data->size);

			temp = strdup("00000000000000000000000000000000");
			for(int i = 32 - s_bits; i < 32 - offset_bits; i++)
				temp[i] = '1';
			
			my_cache_data->index_mask = (int)bin_to_dec(temp);
			my_cache_data->index_mask_offset = offset_bits;

			// Calculating tag_mask and offset
			temp = strdup("00000000000000000000000000000000");
			for(int i = 0; i < 32 - s_bits; i++)
				temp[i] = '1';
			
			my_cache_data->tag_mask = (int)bin_to_dec(temp);
			my_cache_data->tag_mask_offset = 32 - s_bits;

			/*Cache statistics data structures */	
			my_cache_stat_data = (cache_stat*)malloc(sizeof(cache_stat));
			my_cache_stat_data->accesses = 0;
			my_cache_stat_data->misses = 0;
			my_cache_stat_data->replacements = 0;
			my_cache_stat_data->demand_fetches = 0;
			my_cache_stat_data->copies_back = 0;


			my_cache_instruction = (cache*)malloc(sizeof(cache));
			my_cache_instruction->n_sets = cache_isize / cache_block_size;
			my_cache_instruction->size = cache_isize;
			my_cache_instruction->associativity = cache_assoc;
			my_cache_instruction->cache_line_array = (cache_line**)malloc(my_cache_instruction->n_sets * sizeof(cache_line*));
			my_cache_instruction->block_size = cache_block_size;
			for (int i = 0; i < my_cache_instruction->n_sets; i++	)
			{
				my_cache_instruction->cache_line_array[i] = (cache_line*)malloc(sizeof(cache_line));
				my_cache_instruction->cache_line_array[i]->dirty = -1;
				my_cache_instruction->cache_line_array[i]->tag = -1;
			}

			// Calculating index_mask and offset
			offset_bits = LOG2(my_cache_instruction->block_size);
			s_bits = LOG2(my_cache_instruction->size);

			temp = strdup("00000000000000000000000000000000");
			for(int i = 32 - s_bits; i < 32 - offset_bits; i++)
				temp[i] = '1';
			
			my_cache_instruction->index_mask = (int)bin_to_dec(temp);
			my_cache_instruction->index_mask_offset = offset_bits;

			// Calculating tag_mask and offset
			temp = strdup("00000000000000000000000000000000");
			for(int i = 0; i < 32 - s_bits; i++)
				temp[i] = '1';
			
			my_cache_instruction->tag_mask = (int)bin_to_dec(temp);
			my_cache_instruction->tag_mask_offset = 32 - s_bits;

			/*Cache statistics data structures */	
			my_cache_stat_instruction = (cache_stat*)malloc(sizeof(cache_stat));
			my_cache_stat_instruction->accesses = 0;
			my_cache_stat_instruction->misses = 0;
			my_cache_stat_instruction->replacements = 0;
			my_cache_stat_instruction->demand_fetches = 0;
			my_cache_stat_instruction->copies_back = 0;
			break;
		
		case FALSE:
			my_cache_data = (cache*)malloc(sizeof(cache));
			my_cache_data->n_sets = cache_usize / cache_block_size;
			my_cache_data->size = cache_usize;
			my_cache_data->associativity = cache_assoc;
			my_cache_data->cache_line_array = (cache_line**)malloc(my_cache_data->n_sets * sizeof(cache_line*));
			my_cache_data->block_size = cache_block_size;
			for (int i = 0; i < my_cache_data->n_sets; i++)
			{
				my_cache_data->cache_line_array[i] = (cache_line*)malloc(sizeof(cache_line));
				my_cache_data->cache_line_array[i]->dirty = -1;
				my_cache_data->cache_line_array[i]->tag = -1;
			}

			// Calculating index_mask and offset
			offset_bits = LOG2(my_cache_data->block_size);
			s_bits = LOG2(my_cache_data->size);

			temp = strdup("00000000000000000000000000000000");
			for(int i = 32 - s_bits; i < 32 - offset_bits; i++)
				temp[i] = '1';
			
			my_cache_data->index_mask = (int)bin_to_dec(temp);
			my_cache_data->index_mask_offset = offset_bits;

			// Calculating tag_mask and offset
			temp = strdup("00000000000000000000000000000000");
			for(int i = 0; i < 32 - s_bits; i++)
				temp[i] = '1';
			
			my_cache_data->tag_mask = (int)bin_to_dec(temp);
			my_cache_data->tag_mask_offset = s_bits;
			printf("s_bits  %d\n", s_bits);
			print_settings();

			/*Cache statistics data structures */	
			my_cache_stat_data = (cache_stat*)malloc(sizeof(cache_stat));
			my_cache_stat_data->accesses = 0;
			my_cache_stat_data->misses = 0;
			my_cache_stat_data->replacements = 0;
			my_cache_stat_data->demand_fetches = 0;
			my_cache_stat_data->copies_back = 0;


			// my_cache_instruction = (cache*)malloc(sizeof(cache));
			// my_cache_instruction->n_sets = 0;
			// my_cache_instruction->size = 0;
			// my_cache_instruction->associativity = 0;
			// my_cache_instruction->block_size = 0;

			// my_cache_instruction->index_mask = 0;
			// my_cache_instruction->index_mask_offset = 0;

			// Calculating tag_mask and offset
			// my_cache_instruction->tag_mask = 0;
			// my_cache_instruction->tag_mask_offset = 0;

			/*Cache statistics data structures */	
			my_cache_stat_instruction = (cache_stat*)malloc(sizeof(cache_stat));
			my_cache_stat_instruction->accesses = 0;
			my_cache_stat_instruction->misses = 0;
			my_cache_stat_instruction->replacements = 0;
			my_cache_stat_instruction->demand_fetches = 0;
			my_cache_stat_instruction->copies_back = 0;
			break;
	}

	
	
}
/************************************************************/

/************************************************************/

// To_do - performaccess()is called once for each iterationof the simulator loop to simulate a single memory reference to the cache
void perform_access(unsigned addr, unsigned access_type)
{
	/* handle an access to the cache */

	printf("%d   %d\n", addr, access_type);

	int tag, index;

	switch(cache_split)
	{
		case TRUE:

			break;

		case FALSE:
			index = (addr & my_cache_data->index_mask) >> my_cache_data->index_mask_offset;

			tag = (addr & my_cache_data->tag_mask) >> my_cache_data->tag_mask_offset;

			cache_line* temp = my_cache_data->cache_line_array[index];

			printf("\tAddr -> %d    Tags -> %d   Line_tag  -> %d  Index -> %d  Dirty -> %d \n", addr, tag, temp->tag, index, temp->dirty);

			if (tag == temp->tag)
			{
				switch(access_type)
				{
					case 0:
						
						if (temp->dirty == 0)
						{

						}	
						else if (temp->dirty == 1)
						{

						}	
						else
						{
							temp->dirty = 0;
						}
						my_cache_stat_data->accesses++;	
						break;
					case 1:
						if (temp->dirty == 0)
						{

							// my_cache_stat_data->replacements++;
						}	
						else if (temp->dirty == 1)
						{
							// my_cache_stat_data->replacements++;
						}	
						else
						{
							temp->dirty = 0;
						}
						my_cache_stat_data->accesses++;			
						temp->dirty = 1;
						break;
					case 2:
						if (temp->dirty == 0)
						{

						}	
						else if (temp->dirty == 1)
						{

						}	
						else
						{
							temp->dirty = 0;
						}
						my_cache_stat_instruction->accesses++;
						break;
				}		
			}
			else
			{
				switch(access_type)
				{
					case 0:
						if (temp->dirty == 0)
						{
							my_cache_stat_data->replacements++;
						}
						else if (temp->dirty == 1)
						{
							my_cache_stat_data->copies_back += my_cache_data->block_size / 4;
							my_cache_stat_data->replacements++;
						}
						else
						{
							temp->dirty = 0;
						}
						my_cache_stat_data->demand_fetches += my_cache_data->block_size / 4;
						my_cache_stat_data->accesses++;
						my_cache_stat_data->misses++;
						break;
					
					case 1:
						if (temp->dirty == 0)
						{

							my_cache_stat_data->replacements++;
						}
						else if (temp->dirty == 1)
						{
							my_cache_stat_data->copies_back += my_cache_data->block_size / 4;
							my_cache_stat_data->replacements++;
						}
						else
						{
							temp->dirty = 0;
						}
						my_cache_stat_data->demand_fetches += my_cache_data->block_size / 4;
						my_cache_stat_data->accesses++;	
						temp->dirty = 1;		
						my_cache_stat_data->misses++;
						break;

					case 2:
						if (temp->dirty == 0)
						{
							my_cache_stat_instruction->replacements++;
						}	
						else if (temp->dirty == 1)
						{
							my_cache_stat_instruction->replacements++;

						}	
						else
						{
							temp->dirty = 0;
						}
						my_cache_stat_instruction->accesses++;
						my_cache_stat_instruction->misses++;
						my_cache_stat_instruction->demand_fetches += my_cache_data->block_size / 4;							
						break;
				}
				temp->tag = tag;
			}

			break;
	}

	printf("Updating %d\n", my_cache_stat_data->replacements);
}
/************************************************************/

/************************************************************/
void flush()
{
	/* flush the cache */
	cache_line* temp;
	for(int i = 0; i < my_cache_data->n_sets; i++)
	{
		temp = my_cache_data->cache_line_array[i];

		if (temp->dirty == 1)
		{
			temp->dirty = 0;
			my_cache_stat_data->copies_back += my_cache_data->block_size / 4;
			continue;
		}

	}

}
/************************************************************/

/************************************************************/
void dump_settings()
{
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
  	printf("  accesses:  %d\n", my_cache_stat_instruction->accesses);
  	printf("  misses:    %d\n", my_cache_stat_instruction->misses);
  	if (!my_cache_stat_instruction->accesses)
    	printf("  miss rate: 0 (0)\n"); 
  	else
    {
		printf("  miss rate: %2.4f (hit rate %2.4f)\n", (float)my_cache_stat_instruction->misses / (float)my_cache_stat_instruction->accesses,
	 	1.0 - (float)my_cache_stat_instruction->misses / (float)my_cache_stat_instruction->accesses);
  	}
	printf("  replace:   %d\n", my_cache_stat_instruction->replacements);

  	printf(" DATA\n");
  	printf("  accesses:  %d\n", my_cache_stat_data->accesses);
  	printf("  misses:    %d\n", my_cache_stat_data->misses);
  	if (!my_cache_stat_data->accesses)
    	printf("  miss rate: 0 (0)\n"); 
  	
	else
    {
		printf("  miss rate: %2.4f (hit rate %2.4f)\n", (float)my_cache_stat_data->misses / (float)my_cache_stat_data->accesses,
	 	1.0 - (float)my_cache_stat_data->misses / (float)my_cache_stat_data->accesses);
  	}
	printf("  replace:   %d\n", my_cache_stat_data->replacements);
  	
  	printf(" TRAFFIC (in words)\n");
  	printf("  demand fetch:  %d\n", my_cache_stat_instruction->demand_fetches + 
	my_cache_stat_data->demand_fetches);
  	printf("  copies back:   %d\n", my_cache_stat_instruction->copies_back +
	my_cache_stat_data->copies_back);
}
/************************************************************/