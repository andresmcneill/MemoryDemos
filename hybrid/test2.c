#include <stdio.h>

int main(void) {

/*
	char arena[1024];
	char *arenahead = arena;
	char *current, *next;
	char **block_ptr;
	char **listhead = (char **) arena;
	char *retptr;

	int j;
	for( j = 0; j < 3; j++ ){
      current = arenahead + ( j * 256 );
      block_ptr = (char **) current;
      next = current + 256;
      *block_ptr = next;
    }
    block_ptr = (char **) next;
    *block_ptr = NULL;

	printf("arena head: %p\n", arena);

	retptr = (char *)listhead;
	listhead = (char **)(*listhead);

	printf("retptr    : %p\n", retptr);
	printf("listhead  : %p\n", listhead);
*/
	printf("%lu\n",sizeof(char *));

	return 0;
}
