
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
static int cache_size_i = DEFAULT_CACHE_SIZE;
static int cache_size_d = DEFAULT_CACHE_SIZE;
static int cache_block_size_i = DEFAULT_CACHE_BLOCK_SIZE;
static int cache_block_size_d = DEFAULT_CACHE_BLOCK_SIZE;
static int words_per_block_i = DEFAULT_CACHE_BLOCK_SIZE / WORD_SIZE;
static int words_per_block_d = DEFAULT_CACHE_BLOCK_SIZE / WORD_SIZE;
static int cache_assoc_i = DEFAULT_CACHE_ASSOC;
static int cache_assoc_d = DEFAULT_CACHE_ASSOC;
static int cache_writeback_i = DEFAULT_CACHE_WRITEBACK;
static int cache_writeback_d = DEFAULT_CACHE_WRITEBACK;
static int cache_writealloc_i = DEFAULT_CACHE_WRITEALLOC;
static int cache_writealloc_d = DEFAULT_CACHE_WRITEALLOC;

/* cache model data structures */
// static Pcache icache;
// static Pcache dcache;
// static cache c1;
// static cache* my_cache;

static cache* my_cache_d;
static cache* my_cache_i;

static cache_stat* my_cache_stat_d;
static cache_stat* my_cache_stat_i;
// static cache_stat cache_stat_inst;
// static cache_stat cache_stat_data;

/************************************************************/
void set_cache_param(int param, int value)
{
	switch (param) 
	{
  		case CACHE_PARAM_SIZE_I:
    		cache_split = TRUE;
    		cache_size_i = value;
    	break;
  		case CACHE_PARAM_SIZE_D:
    		cache_split = TRUE;
    		cache_size_d = value;
    	break;
  		case CACHE_PARAM_ASSOC_I:
    		cache_assoc_i = value;
    	break;
  		case CACHE_PARAM_ASSOC_D:
    		cache_assoc_d = value;
    	break;
  		case CACHE_PARAM_BLOCK_SIZE_I:
    		cache_block_size_i = value;
    		words_per_block_i = value / WORD_SIZE;
    	break;
  		case CACHE_PARAM_BLOCK_SIZE_D:
    		cache_block_size_d = value;
    		words_per_block_d = value / WORD_SIZE;
    	break;
  		case CACHE_PARAM_WRITETHROUGH_I:
    		if (value == 0)
			{
				cache_writeback_i = TRUE;
				cache_writealloc_i = TRUE;
			}
			else
			{
				cache_writeback_i = FALSE;
				cache_writealloc_i = FALSE;
			}
    	break;
  		case CACHE_PARAM_WRITETHROUGH_D:
			if (value == 0)
			{
				cache_writeback_d = TRUE;
				cache_writealloc_d = TRUE;
			}
			else
			{
				cache_writeback_d = FALSE;
				cache_writealloc_d = FALSE;
			}
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

  	printf("  Associativity: \t%d\n", my_cache_d->associativity);
  	printf("  Block size: \t%d\n", my_cache_d->block_size);
  	printf("  N_Sets: \t%d\n", my_cache_d->n_sets);
  	printf("  Tag_mask: \t%d\n", my_cache_d->tag_mask);
  	printf("  Tag_mask_offset: \t%d\n", my_cache_d->tag_mask_offset);
	printf("  Index_mask: \t%d\n", my_cache_d->index_mask);
  	printf("  Index_mask_offset: \t%d\n", my_cache_d->index_mask_offset);
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

	my_cache_d = (cache*)malloc(sizeof(cache));
	my_cache_d->size = cache_size_d;
	my_cache_d->lines = cache_size_d / cache_block_size_d;
	my_cache_d->associativity = cache_assoc_d;
	my_cache_d->n_sets = my_cache_d->lines / my_cache_d->associativity;
	my_cache_d->cache_line_array = (cache_line**)malloc(my_cache_d->lines * sizeof(cache_line*));
	my_cache_d->block_size = cache_block_size_d;
	for (int i = 0; i < my_cache_d->lines; i++)
	{
		my_cache_d->cache_line_array[i] = (cache_line*)malloc(sizeof(cache_line));
		my_cache_d->cache_line_array[i]->dirty = -1;
		my_cache_d->cache_line_array[i]->tag = -1;
		my_cache_d->cache_line_array[i]->data = calloc(32, sizeof(char));
	}

	// Calculating set_mask and offset
	offset_bits = LOG2(my_cache_d->block_size);
	s_bits = LOG2(my_cache_d->n_sets) + offset_bits;

	temp = strdup("00000000000000000000000000000000");
	for(int i = 32 - s_bits; i < 32 - offset_bits; i++)
		temp[i] = '1';
	
	my_cache_d->set_mask = (int)bin_to_dec(temp);
	my_cache_d->set_mask_offset = offset_bits;

	// Calculating tag_mask and offset
	temp = strdup("00000000000000000000000000000000");
	for(int i = 0; i < 32 - s_bits; i++)
		temp[i] = '1';
	
	my_cache_d->tag_mask = (int)bin_to_dec(temp);
	my_cache_d->tag_mask_offset = 32 - s_bits;

	/*Cache statistics data structures */	
	my_cache_stat_d = (cache_stat*)malloc(sizeof(cache_stat));
	my_cache_stat_d->accesses = 0;
	my_cache_stat_d->misses = 0;
	my_cache_stat_d->replacements = 0;
	my_cache_stat_d->demand_fetches = 0;
	my_cache_stat_d->copies_back = 0;


	my_cache_i = (cache*)malloc(sizeof(cache));
	my_cache_i->size = cache_isize;
	my_cache_i->lines = cache_isize / cache_block_size;
	my_cache_i->associativity = cache_assoc;
	my_cache_i->n_sets = my_cache_i->lines / my_cache_i->associativity;
	my_cache_i->cache_line_array = (cache_line**)malloc(my_cache_i->n_sets * sizeof(cache_line*));
	my_cache_i->block_size = cache_block_size;
	for (int i = 0; i < my_cache_i->lines; i++	)
	{
		my_cache_i->cache_line_array[i] = (cache_line*)malloc(sizeof(cache_line));
		my_cache_i->cache_line_array[i]->dirty = -1;
		my_cache_i->cache_line_array[i]->tag = -1;
		my_cache_i->cache_line_array[i]->data = calloc(32, sizeof(char));
		
	}

	// Calculating index_mask and offset
	offset_bits = LOG2(my_cache_i->block_size);
	s_bits = LOG2(my_cache_i->n_sets) + offset_bits;

	temp = strdup("00000000000000000000000000000000");
	for(int i = 32 - s_bits; i < 32 - offset_bits; i++)
		temp[i] = '1';
	
	my_cache_i->set_mask = (int)bin_to_dec(temp);
	my_cache_i->set_mask_offset = offset_bits;

	// Calculating tag_mask and offset
	temp = strdup("00000000000000000000000000000000");
	for(int i = 0; i < 32 - s_bits; i++)
		temp[i] = '1';
	
	my_cache_i->tag_mask = (int)bin_to_dec(temp);
	my_cache_i->tag_mask_offset = 32 - s_bits;

	/*Cache statistics data structures */	
	my_cache_stat_i = (cache_stat*)malloc(sizeof(cache_stat));
	my_cache_stat_i->accesses = 0;
	my_cache_stat_i->misses = 0;
	my_cache_stat_i->replacements = 0;
	my_cache_stat_i->demand_fetches = 0;
	my_cache_stat_i->copies_back = 0;
	
	
}
/************************************************************/

/************************************************************/

// To_do - performaccess()is called once for each iterationof the simulator loop to simulate a single memory reference to the cache
char* perform_access(unsigned addr, unsigned access_type, unsigned type, char* store)
{
	/* handle an access to the cache */

	// printf("%d   %d\n", addr, access_type);

	int tag, set_index;
	int start_index, end_index;
	int hit;
	char* ans;
	ans = strdup("00000000000000000000000000000000");

	switch(type)
	{
		case DATA:
			set_index = (addr & my_cache_d->set_mask) >> my_cache_d->set_mask_offset;
			tag = (addr & my_cache_d->tag_mask) >> my_cache_d->tag_mask_offset;

			start_index = set_index * my_cache_d->associativity;
			end_index = start_index + my_cache_d->associativity;

			hit = 0;
			cache_line* temp;
			for(int i = start_index; i < end_index; i++)
			{
				temp = my_cache_d->cache_line_array[i];
				if (tag == temp->tag)
					hit = 1, break;
			}

			my_cache_stat_d->accesses++;	
			if (hit == 1)
			{
				switch(type)
				{
					case 0:							//Load data from memory
						switch(cache_writeback_d)
						{
							case TRUE:				// Also means write_alloc is used
								if (temp->dirty == 0)
								{
									
								}
								else if (temp->dirty == 1)
								{

								}
								else
								{
									//Never occurs
								}
								ans = strdup(temp->data);
								my_cache_stat_d->accesses++;	

							break;

							case FALSE:
								// No dirty option needed in write through cache
								ans = strdup(temp->data);
								my_cache_stat_d->accesses++;	

							break;
						}
					break;

					case 1:							//Store data to memory
						switch(cache_writeback_d)
						{
							case TRUE:
								// Doesn't matter if dirty or not as hit is there, only update the contents of the cache line.
								if (temp->dirty == 0)
								{
									
								}
								else if (temp->dirty == 1)
								{
								
								}
								else
								{
									//Never occurs
								}
								temp->data = strdup(store);
								ans = strdup(temp->data);
								temp->dirty = 1;

							break;

							case FALSE:
								// As write_through so update memory and cache at the same time.

								// Assuming that to update cache we need to fetch from memory.
								my_cache_stat_data->demand_fetches += my_cache_data->block_size / WORD_SIZE; 

								// Write the incoming data to memory.
								my_cache_stat_data->copies_back += my_cache_data->block_size / 4;
								memory[addr] = strdup(store);

								temp->data = strdup(store);
								ans = strdup(temp->data);
								temp->dirty = 1;								

							break;
						}
					break;
				}
			}
			else 				// Miss
			{
				switch(type)
				{
					case 0:							//Load data from memory
						switch(cache_writeback_d)
						{
							case TRUE:				// Also means write_alloc is used
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
									temp->dirty = 0; // Should not happen as loading without storing is not defined.
								}
								temp->data = strdup(memory[addr]);
								ans = strdup(temp->data);
								my_cache_stat_d->misses++;


							break;

							case FALSE:
								// No dirty option needed in write through cache
								ans = strdup(temp->data);
								my_cache_stat_d->accesses++;	

							break;
						}
					break;

					case 1:							//Store data to memory

					break;
				}
			}

		break;

		case INSTRUCTION:
			
			set_index = (addr & my_cache_i->set_mask) >> my_cache_i->set_mask_offset;
			tag = (addr & my_cache_i->tag_mask) >> my_cache_i->tag_mask_offset;

			start_index = set_index * my_cache_i->associativity;
			end_index = start_index + my_cache_i->associativity;

			hit = 0;
			cache_line* temp;
			for(int i = start_index; i < end_index; i++)
			{
				temp = my_cache_i->cache_line_array[i];
				if (tag == temp->tag)
					hit = 1, break;
			}

			if (hit == 1)
			{
				switch(type)
				{
					case 0:							//Load data from memory

					break;

					case 1:							//Store data to memory

					break;
				}
			}
			else
			{
				switch(type)
				{
					case 0:							//Load data from memory

					break;

					case 1:							//Store data to memory

					break;
				}
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
	for(int i = 0; i < my_cache_d->n_sets; i++)
	{
		temp = my_cache_d->cache_line_array[i];

		if (temp->dirty == 1)
		{
			temp->dirty = 0;
			my_cache_stat_data->copies_back += my_cache_d->block_size / 4;
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
    	("  D-cache size: \t%d\n", cache_size_d);
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