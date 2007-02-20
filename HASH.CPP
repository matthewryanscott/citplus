#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

typedef unsigned int uint;

/* -------------------------------------------------------------------- */
/*	hash()			Make an int out of a string							*/
/* -------------------------------------------------------------------- */
uint hash(const char *str)
	{
	int h, shift;

	for (h = shift = 0; *str; shift = (shift + 1) & 7, str++)
		{
		h ^= (toupper(*str)) << shift;
		}

	return (h);
	}


void main(int argc, char **argv)
	{
	if (argc > 1)
		{
		printf("%s: %u\n", argv[1], hash(argv[1]));
		}
	}