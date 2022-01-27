#include <stdio.h>

int main()
{
	printf("%d", 1 < 10 && 10 <= 10);
	printf("%d", 1 < 10 || 0 > 10);
	printf("%d", 1 < 2 < 3 && 2 >= 1 >= 1);
	printf("%d", 2 < 2 < 3 || 2 >= 1 >= 1);
}