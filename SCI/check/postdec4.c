#include <stdio.h>

int main()
{
	int a, b;
	a = 0;
	b = (a-- * 10) + 1;
	printf("%d", b);
}