
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
#include "processor.h"

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


// Global counter to search the least recent used block
int least_recent;


/************************************************************/
void set_cache_param(int param, int value)
{
	printf("IN setgging ------  %d, %d\n", param, value);
	switch (param) 
	{
  		case CACHE_PARAM_SIZE_I:
    		cache_size_i = value * 1024;
    	break;
  		case CACHE_PARAM_SIZE_D:
    		cache_size_d = value * 1024;
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
	printf("*** CACHE PARAMETERS ***\n");

	printf("Instruction Cache\n");
  	printf("\tSize: \t%d\n", cache_size_i);
  	printf("\tAssociativity: \t%d\n", cache_assoc_i);
  	printf("\tBlock size: \t%d\n", cache_block_size_i);	  
  	printf("\tWriteback: \t%d\n", cache_writeback_i);

	printf("Data Cache\n");
  	printf("\tSize: \t%d\n", cache_size_d);
  	printf("\tAssociativity: \t%d\n", cache_assoc_d);
  	printf("\tBlock size: \t%d\n", cache_block_size_d);	  
  	printf("\tWriteback: \t%d\n", cache_writeback_d);

}
void init_cache()
{
	/* initialize the cache.*/
	int offset_bits;
	int s_bits;
	char* temp;
	least_recent = 0;

	my_cache_d = (cache*)malloc(sizeof(cache));
	my_cache_d->size = cache_size_d;
	my_cache_d->lines = cache_size_d / cache_block_size_d;
	my_cache_d->associativity = cache_assoc_d;
	my_cache_d->n_sets = my_cache_d->lines / my_cache_d->associativity;
	my_cache_d->cache_line_array = (cache_line**)malloc(my_cache_d->lines * sizeof(cache_line*));
	my_cache_d->block_size = cache_block_size_d;
	// dump_settings();
	for (int i = 0; i < my_cache_d->lines; i++)
	{
		my_cache_d->cache_line_array[i] = (cache_line*)malloc(sizeof(cache_line));
		my_cache_d->cache_line_array[i]->dirty = -1;
		my_cache_d->cache_line_array[i]->tag = -1;
		my_cache_d->cache_line_array[i]->data = (char*)malloc(cache_block_size_d * 4 * sizeof(char));
		my_cache_d->cache_line_array[i]->used = 0;		
	}

	// Calculating set_mask and offset
	printf("fuck %d  %d  %d  %d \n", my_cache_d->size, my_cache_d->n_sets, my_cache_d->lines, s_bits);
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


	if(offset_bits == 0)
	{
		// Calculating byte_mask and offset
		
		my_cache_d->byte_mask = 0;
		my_cache_d->byte_mask_offset = 0;

		// Calculating byte_mask and offset		
		my_cache_d->word_mask = 0;
		my_cache_d->word_mask_offset = 0;
	}
	else if(offset_bits == 1)
	{
		// Calculating byte_mask and offset
		my_cache_d->byte_mask = 1;
		my_cache_d->byte_mask_offset = 0;

		// Calculating byte_mask and offset		
		my_cache_d->word_mask = 0;
		my_cache_d->word_mask_offset = 0;
	}
	else if(offset_bits == 2)
	{
		// Calculating byte_mask and offset
		temp = strdup("00000000000000000000000000000011");
		
		my_cache_d->byte_mask = (int)bin_to_dec(temp);
		my_cache_d->byte_mask_offset = 0;

		// Calculating byte_mask and offset		
		my_cache_d->word_mask = 0;
		my_cache_d->word_mask_offset = 0;

	}
	else
	{
		// Calculating byte_mask and offset
		temp = strdup("00000000000000000000000000000011");
		
		my_cache_d->byte_mask = (int)bin_to_dec(temp);
		my_cache_d->byte_mask_offset = 0;

		// Calculating byte_mask and offset
		temp = strdup("00000000000000000000000000000000");
		for(int i = 32 - offset_bits; i < 30; i++)
			temp[i] = '1';
		
		my_cache_d->word_mask = (int)bin_to_dec(temp);
		my_cache_d->word_mask_offset = 2;

	}
	
	/*Cache statistics data structures */	
	my_cache_stat_d = (cache_stat*)malloc(sizeof(cache_stat));
	my_cache_stat_d->accesses = 0;
	my_cache_stat_d->misses = 0;
	my_cache_stat_d->replacements = 0;
	my_cache_stat_d->demand_fetches = 0;
	my_cache_stat_d->copies_back = 0;


	my_cache_i = (cache*)malloc(sizeof(cache));
	my_cache_i->size = cache_size_i;
	my_cache_i->lines = cache_size_i / cache_block_size_i;
	my_cache_i->associativity = cache_assoc_i;
	my_cache_i->n_sets = my_cache_i->lines / my_cache_i->associativity;
	my_cache_i->cache_line_array = (cache_line**)malloc(my_cache_i->n_sets * sizeof(cache_line*));
	my_cache_i->block_size = cache_block_size_i;
	for (int i = 0; i < my_cache_i->lines; i++	)
	{
		my_cache_i->cache_line_array[i] = (cache_line*)malloc(sizeof(cache_line));
		my_cache_i->cache_line_array[i]->dirty = -1;
		my_cache_i->cache_line_array[i]->tag = -1;
		my_cache_i->cache_line_array[i]->data = (char*)malloc(cache_block_size_i * 4 * sizeof(char));
		my_cache_i->cache_line_array[i]->used = 0;				
		
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

	if(offset_bits == 0)
	{
		// Calculating byte_mask and offset
		
		my_cache_i->byte_mask = 0;
		my_cache_i->byte_mask_offset = 0;

		// Calculating byte_mask and offset		
		my_cache_i->word_mask = 0;
		my_cache_i->word_mask_offset = 0;
	}
	else if(offset_bits == 1)
	{
		// Calculating byte_mask and offset
		my_cache_i->byte_mask = 1;
		my_cache_i->byte_mask_offset = 0;

		// Calculating byte_mask and offset		
		my_cache_i->word_mask = 0;
		my_cache_i->word_mask_offset = 0;
	}
	else if(offset_bits == 2)
	{
		// Calculating byte_mask and offset
		temp = strdup("00000000000000000000000000000011");
		
		my_cache_i->byte_mask = (int)bin_to_dec(temp);
		my_cache_i->byte_mask_offset = 0;

		// Calculating byte_mask and offset		
		my_cache_i->word_mask = 0;
		my_cache_i->word_mask_offset = 0;

	}
	else
	{
		// Calculating byte_mask and offset
		temp = strdup("00000000000000000000000000000011");
		
		my_cache_i->byte_mask = (int)bin_to_dec(temp);
		my_cache_i->byte_mask_offset = 0;

		// Calculating byte_mask and offset
		temp = strdup("00000000000000000000000000000000");
		for(int i = 32 - offset_bits; i < 30; i++)
			temp[i] = '1';
		
		my_cache_i->word_mask = (int)bin_to_dec(temp);
		my_cache_i->word_mask_offset = 2;

	}
	

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
// For hit 
char* load_from_cache(int byte_index, int word_index, cache_line* from)
{
	char* temp = calloc(32, sizeof(char));
	char* ans = calloc(8, sizeof(char));

	temp = slice(from->data + word_index * 32, 32);
	ans = slice(temp + byte_index * 8, 8);

	return ans;
}	

void write_to_cache(int byte_index, int word_index, cache_line* to, char* data_store)
{
	char* ans;
	ans = strdup(to->data);
	for(int i = 0; i < 8; i++)
	{
		ans[i + word_index * 32 + byte_index * 8] = data_store[i];
	}
	to->data = strdup(ans);
}	


// For miss
void cache_to_memory_write(int set_idx, cache_line* to_rep, int type)
{
	int addr;
	int total;
	switch(type)
	{
		case 0:		// Data type
			addr = set_idx << my_cache_d->set_mask_offset;
			addr += to_rep->tag << my_cache_d->tag_mask_offset;
			total = my_cache_d->block_size;
			for(int i = 0; i < total; i++)
				memory[i + addr] = slice(to_rep->data + i * 8, 8);

		break;

		case 1:
			addr = set_idx << my_cache_i->set_mask_offset;
			addr += to_rep->tag << my_cache_i->tag_mask_offset;
			total = my_cache_i->block_size;
			for(int i = 0; i < total; i++)
				memory[i + addr] = slice(to_rep->data + i * 8, 8);

		break;
	}
}

void memory_to_cache_load(int set_idx, int tag, cache_line* to_rep, int type)
{
	int addr;
	int total;
	char* ans;
	switch(type)
	{
		case 0:		// Data type
			ans = calloc(my_cache_d->block_size * 8, sizeof(char));
			addr = set_idx << my_cache_d->set_mask_offset;
			addr += tag << my_cache_d->tag_mask_offset;
			total = my_cache_d->block_size;
			ans = strdup(memory[addr]);
			for(int i = 1; i < total; i++)
				ans = concat(ans, memory[i + addr]);

		break;

		case 1:		// Instruction type.		
			ans = calloc(my_cache_i->block_size * 8, sizeof(char));
			addr = set_idx << my_cache_i->set_mask_offset;
			addr += tag << my_cache_i->tag_mask_offset;
			total = my_cache_i->block_size;
			ans = strdup(memory[addr]);
			for(int i = 1; i < total; i++)
				ans = concat(ans, memory[i + addr]);

		break;
	}

	to_rep->data = strdup(ans);
}

char* access_miss_help(int byte_index, int word_index, char* data_load, char* data_store, int type)
{
	char* temp = calloc(32, sizeof(char));
	char* ans = calloc(8, sizeof(char));

	switch(type)
	{
		case 0:											// Load from cache_line
			temp = slice(data_load + word_index * 32, 32);
			ans = slice(temp + byte_index * 8, 8);

		break;

		case 1:											// Store to cache line
			ans = strdup(data_store);
			for(int i = word_index * 32; i < 8; i++)
			{
				ans[i + word_index * 32 + byte_index * 8] = data_load[i];
			}

		break;
	}
	return ans;
}	


// To_do - performaccess()is called once for each iterationof the simulator loop to simulate a single memory reference to the cache
char* perform_access(unsigned addr, unsigned access_type, unsigned type, char* store)
{
	/* handle an access to the cache */

	// printf("%d   %d\n", addr, access_type);
	least_recent++;
	int tag, set_index;
	int byte_index, word_index;
	int start_index, end_index;
	int hit;
	char* ans;
	ans = strdup("00000000000000000000000000000000");
	cache_line* temp;
	cache_line* to_replace;
	int mini;
	switch(type)
	{
		case DATA:
			set_index = (addr & my_cache_d->set_mask) >> my_cache_d->set_mask_offset;
			tag = (addr & my_cache_d->tag_mask) >> my_cache_d->tag_mask_offset;

			byte_index = (addr & my_cache_d->byte_mask) >> my_cache_d->byte_mask_offset;
			word_index = (addr & my_cache_d->word_mask) >> my_cache_d->word_mask_offset;

			start_index = set_index * my_cache_d->associativity;
			end_index = start_index + my_cache_d->associativity;

			hit = 0;
			mini = 10;
			for(int i = start_index; i < end_index; i++)
			{
				temp = my_cache_d->cache_line_array[i];
				if (tag == temp->tag)
					hit = 1;
				
				if (temp->used < mini)
				{
					mini = temp->used;
					to_replace = temp;
				}

			}

			my_cache_stat_d->accesses++;	
			if (hit == 1)
			{
				temp->tag = tag;
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
								ans = load_from_cache(byte_index, word_index, temp);

							break;

							case FALSE:
								// No dirty option needed in write through cache
								ans = load_from_cache(byte_index, word_index, temp);

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
								write_to_cache(byte_index, word_index, temp, store);
								ans = strdup(store);
								temp->dirty = 1;

							break;

							case FALSE:
								// As write_through so update memory and cache at the same time.

								// Assuming that to update cache we need to fetch from memory.
								my_cache_stat_d->demand_fetches += my_cache_d->block_size / WORD_SIZE; 

								// Write the incoming data to memory.
								my_cache_stat_d->copies_back += my_cache_d->block_size / WORD_SIZE;
								
								// Can't as memdump will give wrong results

								write_to_cache(byte_index, word_index, temp, store);
								memory[addr] = strdup(store);		// Store diretly for storing in no write allocate and writethrough.
								ans = strdup(store);
								temp->dirty = 1;								

							break;
						}
					break;
				}
			}
			else 				// Miss
			{
				// my_cache_stat_d->demand_fetches += my_cache_d->block_size / WORD_SIZE; // Compulsory for all in misses.
				my_cache_stat_d->misses++;
				switch(type)
				{
					case 0:							//Load data from memory
						switch(cache_writeback_d)
						{
							case TRUE:				// Also means write_alloc is used

								// Perhaps not needed as memory is written already
								cache_to_memory_write(set_index, to_replace, 0); // Write back to memory first.
								memory_to_cache_load(set_index, tag, to_replace, 0);
								ans = strdup(memory[addr]);

								my_cache_stat_d->replacements++;									
								my_cache_stat_d->copies_back += my_cache_d->block_size / WORD_SIZE;
								
								// if (temp->dirty == 0)
								// {
								// 	my_cache_stat_d->replacements++;									
								// }
								// else if (temp->dirty == 1)
								// {
								// 	my_cache_stat_d->copies_back += my_cache_d->block_size / WORD_SIZE;
								// 	my_cache_stat_d->replacements++;
								// }
								// else
								// {
								// 	temp->dirty = 0; // Should not happen as loading without storing is not defined.
								// }
								// temp->data = strdup(memory[addr]);
								// ans = strdup(temp->data);

							break;

							case FALSE:
								// Data at the missed-write location is not loaded to cache.
								// TO_DO confusion
								cache_to_memory_write(set_index, to_replace, 0); // Write back to memory first.
								memory_to_cache_load(set_index, tag, to_replace, 0);
								ans = strdup(memory[addr]);

								my_cache_stat_d->replacements++;									
								my_cache_stat_d->copies_back += my_cache_d->block_size / WORD_SIZE;
								
							break;
						}
					break;

					case 1:							//Store data to memory
						switch(cache_writeback_d)
						{
							case TRUE: 	// Write allocate 
								// Doesn't matter if dirty or not as hit is there, only update the contents of the cache line.
								
								// Perhaps not needed as memory is already written.
								cache_to_memory_write(set_index, to_replace, 0); // Write back to memory first.
								write_to_cache(byte_index, word_index, temp, store);
								
								ans = strdup(store);

								my_cache_stat_d->replacements++;									
								my_cache_stat_d->copies_back += my_cache_d->block_size / WORD_SIZE;
								temp->dirty = 1;

							break;

							case FALSE:

								memory[addr] = strdup(store);
								// my_cache_stat_d->demand_fetches += my_cache_d->block_size / WORD_SIZE; 

								my_cache_stat_d->copies_back += my_cache_d->block_size / 4;
								ans = strdup(store);

							break;
						}
					break;
				}
				to_replace->tag = tag;
			}

		break;

		case INSTRUCTION:
			
			set_index = (addr & my_cache_i->set_mask) >> my_cache_i->set_mask_offset;
			tag = (addr & my_cache_i->tag_mask) >> my_cache_i->tag_mask_offset;

			start_index = set_index * my_cache_i->associativity;
			end_index = start_index + my_cache_i->associativity;

			hit = 0;
			my_cache_stat_d->accesses++;	
			
			for(int i = start_index; i < end_index; i++)
			{
				temp = my_cache_i->cache_line_array[i];
				if (tag == temp->tag)
				{
					hit = 1;
					break;
				}
			}

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

							break;

							case FALSE:
								// No dirty option needed in write through cache
								ans = strdup(temp->data);

							break;
						}
					break;

					case 1:							//Store data to memory
						// Never should come.
					break;
				}
			}
			else
			{
				my_cache_stat_i->demand_fetches += my_cache_d->block_size / WORD_SIZE; // Compulsory for all in misses.
				my_cache_stat_i->misses++;
				switch(type)
				{
					case 0:							//Load data from memory
						switch(cache_writeback_d)
						{
							case TRUE:				// Also means write_alloc is used
								if (temp->dirty == 0)
								{
									my_cache_stat_i->replacements++;									
								}
								else if (temp->dirty == 1)
								{
									my_cache_stat_i->copies_back += my_cache_i->block_size / WORD_SIZE;
									my_cache_stat_i->replacements++;
								}
								else
								{
									temp->dirty = 0; // Should not happen as loading without storing is not defined.
								}
								temp->data = strdup(memory[addr]);
								ans = strdup(temp->data);

							break;

							case FALSE:
								// No dirty option needed in write through cache, no write_alloc is used.
								// Data at the missed-write location is not loaded to cache.

								ans = strdup(temp->data);

							break;
						}
					break;

					case 1:							//Store data to memory
						// Never should come.
					break;
				}
			}

		break;
	
	}

	return ans;
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
			my_cache_stat_d->copies_back += my_cache_d->block_size / 4;
			continue;
		}

	}

}
/************************************************************/

/************************************************************/
void dump_settings()
{
	printf("*** CACHE SETTINGS ***\n");

	printf("Instruction Cache\n");
  	printf("\tAssociativity: \t%d\n", my_cache_i->associativity);
  	printf("\tBlock size: \t%d\n", my_cache_i->block_size);
  	printf("\tN_Sets: \t%d\n", my_cache_i->n_sets);
  	printf("\tTag_mask: \t%d\n", my_cache_i->tag_mask);
  	printf("\tTag_mask_offset: \t%d\n", my_cache_i->tag_mask_offset);
	printf("\tSet_mask: \t%d\n", my_cache_i->set_mask);
  	printf("\tSet_mask_offset: \t%d\n", my_cache_i->set_mask_offset);
	printf("\tByte_mask: \t%d\n", my_cache_i->byte_mask);
  	printf("\tByte_mask_offset: \t%d\n", my_cache_i->byte_mask_offset);
	printf("\tWord_mask: \t%d\n", my_cache_i->word_mask);
  	printf("\tWord_mask_offset: \t%d\n", my_cache_i->word_mask_offset);
	  

	printf("Data Cache\n");
  	printf("\tAssociativity: \t%d\n", my_cache_d->associativity);
  	printf("\tBlock size: \t%d\n", my_cache_d->block_size);
  	printf("\tN_Sets: \t%d\n", my_cache_d->n_sets);
  	printf("\tTag_mask: \t%d\n", my_cache_d->tag_mask);
  	printf("\tTag_mask_offset: \t%d\n", my_cache_d->tag_mask_offset);
	printf("\tSet_mask: \t%d\n", my_cache_d->set_mask);
  	printf("\tSet_mask_offset: \t%d\n", my_cache_d->set_mask_offset);
	printf("\tByte_mask: \t%d\n", my_cache_d->byte_mask);
  	printf("\tByte_mask_offset: \t%d\n", my_cache_d->byte_mask_offset);
	printf("\tWord_mask: \t%d\n", my_cache_d->word_mask);
  	printf("\tWord_mask_offset: \t%d\n", my_cache_d->word_mask_offset);
	
}
// /************************************************************/

// /************************************************************/
void print_stats()
{
	printf("\n*** CACHE STATISTICS ***\n");

  	printf(" INSTRUCTIONS\n");
  	printf("  accesses:  %d\n", my_cache_stat_i->accesses);
  	printf("  misses:    %d\n", my_cache_stat_i->misses);
  	if (!my_cache_stat_i->accesses)
    	printf("  miss rate: 0 (0)\n"); 
  	else
    {
		printf("  miss rate: %2.4f (hit rate %2.4f)\n", (float)my_cache_stat_i->misses / (float)my_cache_stat_i->accesses,
	 	1.0 - (float)my_cache_stat_i->misses / (float)my_cache_stat_i->accesses);
  	}
	printf("  replace:   %d\n", my_cache_stat_i->replacements);

  	printf(" DATA\n");
  	printf("  accesses:  %d\n", my_cache_stat_d->accesses);
  	printf("  misses:    %d\n", my_cache_stat_d->misses);
  	if (!my_cache_stat_d->accesses)
    	printf("  miss rate: 0 (0)\n"); 
  	
	else
    {
		printf("  miss rate: %2.4f (hit rate %2.4f)\n", (float)my_cache_stat_d->misses / (float)my_cache_stat_d->accesses,
	 	1.0 - (float)my_cache_stat_d->misses / (float)my_cache_stat_d->accesses);
  	}
	printf("  replace:   %d\n", my_cache_stat_d->replacements);
  	
  	printf(" TRAFFIC (in words)\n");
  	printf("  demand fetch:  %d\n", my_cache_stat_i->demand_fetches + 
	my_cache_stat_d->demand_fetches);
  	printf("  copies back:   %d\n", my_cache_stat_i->copies_back +
	my_cache_stat_d->copies_back);
}
/************************************************************/