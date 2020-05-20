#include "hybrid.h"

void init_mem_alloc(){
  char *current, *next, **block_ptr;
  int i, j;

  arena_head[0] = level_0_arena;
  arena_head[1] = level_1_arena;
  arena_head[2] = level_2_arena;

  arena_size[0] = ARENA_0_SIZE;
  arena_size[1] = ARENA_1_SIZE;
  arena_size[2] = ARENA_2_SIZE;

  arena_block_size[0] = ARENA_0_BLOCK_SIZE;
  arena_block_size[1] = ARENA_1_BLOCK_SIZE;
  arena_block_size[2] = ARENA_2_BLOCK_SIZE;

  arena_count[0] = ARENA_0_SIZE/ARENA_0_BLOCK_SIZE;
  arena_count[1] = ARENA_1_SIZE/ARENA_1_BLOCK_SIZE;
  arena_count[2] = ARENA_2_SIZE/ARENA_2_BLOCK_SIZE;

  for( i = 0; i < NUM_BITMAP_WORDS; i++ ){ bitmap[i] = 0; }

  for( i = 1; i < NUM_ARENAS; i++ ){
    for( j = 0; j < ( arena_count[i] - 1 ); j++ ){
      current = arena_head[i] + ( j * arena_block_size[i] );
      block_ptr = (char **) current;
      next = current + arena_block_size[i];
      *block_ptr = next;
    }
    block_ptr = (char **) next;
    *block_ptr = NULL;
  }

  min_address = arena_head[0];
  max_address = arena_head[0] + arena_size[0];
  for( i = 1; i < NUM_ARENAS; i++ ){
    if( arena_head[i] < min_address ) min_address = arena_head[i];
    if( ( arena_head[i] + arena_size[i] ) > max_address )
      max_address = arena_head[i] + arena_size[i];
  }

  for( i = 0; i < NUM_ARENAS; i++ ){
    printf( "arena %i base address = %p with blocksize 0x%x (= %d)\n",
      i, arena_head[i], arena_block_size[i], arena_block_size[i] );
  }
}

void display_arenas(){
  char **block_ptr;
  int i, j;
  printf( "---- arena counts for available blocks ----\n" );
  for( i = 0; i < NUM_ARENAS; i++ ){
    printf( "%6d blocks available in arena %d\n", arena_count[i], i );
  }

  /* extra verification for non-bitmapped arenas */
  for( i = 1; i < NUM_ARENAS; i++ ){
      j = 0; block_ptr = (char **) arena_head[i];
      while( block_ptr != NULL ){ j++; block_ptr = (char **)(*block_ptr); }
      if( j != arena_count[i] ){
        printf( "mismatched counts for arena %d: %d versus %d\n",
          i, j, arena_count[i] );
      }
    }

  printf( "  bitmap words\n" );
  for( i = 0; i < NUM_BITMAP_WORDS; i++ ){
    printf( "  0x%08x", bitmap[i] );
  }
  printf( "\n" );

  printf( "---- end ----\n" );
}

int main(){
  char *p[64];
  int i;

  init_mem_alloc();
  display_arenas();

  p[0] = allocate(   -1 ); printf( "ask for   -1, get %p\n", p[0] );
  p[0] = allocate(    0 ); printf( "ask for    0, get %p\n", p[0] );
  for( i = 0; i < 20; i++ ){
    p[i] = allocate( i + 1 );
    printf( "ask for %4d, get %p\n", i + 1, p[i] );
  }
  display_arenas();
  for( i = 0; i < 20; i++ ){
    p[i+20] = allocate( i + 21 );
    printf( "ask for %4d, get %p\n", i + 21, p[i+20] );
  }
  display_arenas();
  release( p[5] ); printf( "release %p\n", p[5] );
  release( p[6] ); printf( "release %p\n", p[6] );
  display_arenas();
  p[40] = allocate(  100 ); printf( "ask for  100, get %p\n", p[40] );
  p[41] = allocate(  100 ); printf( "ask for  100, get %p\n", p[41] );
  printf( "attempt to release bad address: 0\n" );
  release( (char *) 0x0 );
  printf( "attempt to release bad address: p[40] + 2\n" );
  release( p[40] + 2 );
  printf( "attempt to release bad address: p[40] + 32\n" );
  release( p[40] + 32 );
  display_arenas();

  p[42] = allocate( 5000 ); printf( "ask for 5000, get %p\n", p[42] );
  p[43] = allocate( 1000 ); printf( "ask for 1000, get %p\n", p[43] );
  p[44] = allocate( 1000 ); printf( "ask for 1000, get %p\n", p[44] );
  p[45] = allocate( 1000 ); printf( "ask for 1000, get %p\n", p[45] );
  p[46] = allocate( 1000 ); printf( "ask for 1000, get %p\n", p[46] );
  release(p[43]);
  release(p[44]);
  release(p[45]);
  release(p[46]);
  display_arenas();
  p[47] = allocate( 1000 ); printf( "ask for 1000, get %p\n", p[47] );

  return 0;
}
