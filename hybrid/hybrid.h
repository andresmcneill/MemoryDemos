#include<stdio.h>
#include<stdlib.h>

#define NUM_ARENAS 3

#define ARENA_0_SIZE 8192
#define ARENA_1_SIZE 8192
#define ARENA_2_SIZE 16384

#define ARENA_0_BLOCK_SIZE 64
#define ARENA_1_BLOCK_SIZE 256
#define ARENA_2_BLOCK_SIZE 4096

#define HEADER_SIGNATURE 0xab51ead0

char /*__attribute__ ((aligned (65536)))*/ level_0_arena[ARENA_0_SIZE];
char /*__attribute__ ((aligned (65536)))*/ level_1_arena[ARENA_1_SIZE];
char /*__attribute__ ((aligned (65536)))*/ level_2_arena[ARENA_2_SIZE];

int arena_size[NUM_ARENAS];
int arena_block_size[NUM_ARENAS];
int arena_count[NUM_ARENAS];

#define NUM_BITMAP_WORDS (ARENA_0_SIZE/(32*ARENA_0_BLOCK_SIZE))
unsigned int bitmap[NUM_BITMAP_WORDS];

char *arena_head[NUM_ARENAS];
char *min_address, *max_address;

char *allocate( int size );
void release( char *release_ptr );
