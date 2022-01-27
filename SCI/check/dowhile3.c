#include <stdio.h>

int main()
{
	int i = 0, j = 0;
	
	do {
		i++;
		do {
			j++;
			printf("%d", j);
		} while (j < 10);
	} while (i < 10);
}