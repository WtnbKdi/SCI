#include <stdio.h>

int main()
{
	int a, b, c;
	a = 0;
	b = 0;
	c = a-- + a-- - b-- + b--;
	printf("%d", c);
}