#include <stdio.h>

int main()
{
	printf("%d", (1 || 0) && !0);
	printf("%d", 1 && 1 && 0);
	printf("%d", 1 && 1 || 0);
}