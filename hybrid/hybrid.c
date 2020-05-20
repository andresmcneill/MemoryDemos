#include <stdio.h>
#include <stdlib.h>
#include "hybrid.h"

char *list_allocate(int arena) {
	char *headptr;
	char *blockptr;
	long long unsigned int header = HEADER_SIGNATURE;
	long long unsigned int *assignheader;

	if (arena == 1) {
		//	no free blocks available, can't allocate
		if (arena_count[1] <= 0) {
			printf("no free memory in arena\n  => no action taken\n");
			return NULL;
		}
		//	find address
		headptr = arena_head[1];
		//	set free list head to next node
		char **nextptr = (char **) arena_head[1];
		nextptr = (char **) *nextptr;
		arena_head[1] = (char *) nextptr;

		//	set header
		header = header | 0x1;
		assignheader = (long long unsigned int *) headptr;
		*assignheader = header;

		//	increment blockptr
		blockptr = headptr + 8;

		//	decrement free count
		arena_count[1]--;
		return blockptr;
	}
	else if (arena == 2) {
		//	no free blocks available, can't allocate
		if (arena_count[2] <= 0) {
			//printf("no free memory in arena\n  => no action taken\n");
			return NULL;
		}
		//	find address
		headptr = arena_head[2];
		//	set free list head to next node
		char **nextptr = (char **) arena_head[2];
		nextptr = (char **) *nextptr;
		arena_head[2] = (char *) nextptr;

		//	set header
		header = header | 0x2;
		assignheader = (long long unsigned int *) headptr;
		*assignheader = header;

		//	increment blockptr
		blockptr = headptr + 8;

		//	decrement free count
		arena_count[2]--;
		return blockptr;
	}
	else {
		printf("invalid arena\n  => no action taken\n");
		return NULL;
	}
}

char *bitmap_allocate(void) {
	//	No free blocks available, can't allocate
	if (arena_count[0] <= 0) {
		return NULL;
	}

	unsigned int bitscan;
	for (int i = 0; i < NUM_BITMAP_WORDS; i++) {
		bitscan = 0x80000000;
		for (int j = 0; j < (ARENA_0_SIZE/ARENA_0_BLOCK_SIZE)/NUM_BITMAP_WORDS; j++) {
			if ((bitscan & bitmap[i]) == 0) {
				long long unsigned int header = HEADER_SIGNATURE;
				long long unsigned int *assignheader;

				char *headptr;
				char *blockptr;
				//	find address
				headptr = arena_head[0] + (j * ARENA_0_BLOCK_SIZE) + (i * 32 * ARENA_0_BLOCK_SIZE);
				assignheader = (long long unsigned int *) headptr;
				blockptr = headptr + 8;

				//	set header
				*assignheader = header;

				//	decrement free count
				arena_count[0]--;

				//	use | on bitmap[i]
				bitmap[i] = bitmap[i] | bitscan;

				return blockptr;
			}
			//	Shift the bitwise iterator
			bitscan = bitscan >> 1;
		}
	}
	//	Couldn't find a free block (this is nonstandard, arena_count[0] should
	//	already be true).
	return NULL;
}

char *allocate( int size ) {
	if( size <= 0 ){
      return NULL;
    }else if( size <= ( arena_block_size[0] - 8 ) ){
      return bitmap_allocate();
    }else if( size <= ( arena_block_size[1] - 8 ) ){
      return list_allocate( 1 );
    }else if( size <= ( arena_block_size[2] - 8 ) ){
      return list_allocate( 2 );
    }else{
      return NULL;
    }
}

void release( char *release_ptr ) {
	char *ptr;
	long long unsigned int header, *header_ptr;

	if( (long long int) release_ptr & 0x7 ){
		printf( "pointer not aligned on 8B boundary in release() function\n" );
		printf( "  => no action taken\n" );
		return;
	}
	if( ( release_ptr < min_address ) || ( release_ptr > max_address )  ){
		printf( "pointer out of range in release() function\n" );
		printf( "  => no action taken\n" );
		return;
	}
	ptr = release_ptr - 8;
	header_ptr = (long long unsigned int *) ptr;
	header = *header_ptr;
	if( ( header & 0xfffffff0 ) != HEADER_SIGNATURE ) {
	    printf( "header does not match in release() function\n" );
	    printf( "  => no action taken\n" );
	    return;
	}

	//	Belongs in arena 0
	if (header == HEADER_SIGNATURE) {
		long offset;
		int bitmap_word;
		int subword_index;
		unsigned int bit = 0x80000000;
		offset = (header_ptr - ((long long unsigned int *) arena_head[0])) / 8;
		bitmap_word = offset / 32;
		subword_index = offset % 32;
		//	Move bit to correct bitmap location
		for (int i = 0; i < subword_index; i++) bit = bit >> 1;
		//	Set corresponding bit to 0
		bitmap[bitmap_word] &= ~(0x1 << (31 - subword_index));
		arena_count[0]++;
		return;
	}
	//	Block in arena 1
	else if ((header & 0x1) == 1) {
		char **newhead = (char **) (release_ptr - 8);
		*newhead = (char *) arena_head[1];
		arena_head[1] = (char *) newhead;
		arena_count[1]++;
		return;
	}
	//	Block in arena 2
	else if ((header & 0x2) == 2) {
		char **newhead = (char **) (release_ptr - 8);
		*newhead = (char *) arena_head[2];
		arena_head[2] = (char *) newhead;
		arena_count[2]++;
		return;
	}
	//	Function signature not recognized
	else {
		printf("header index not recognized in release() function\n");
		printf("  => no action taken\n");
		return;
	}
}
