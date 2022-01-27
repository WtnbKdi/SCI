#include <stdio.h>

int main()
{
	int a;
	a = 0;

	a += 10;
	printf("%d", a);
	a -= 9;
	printf("%d", a);
	a *= 10;
	printf("%d", a);
	a /= 10;
	printf("%d", a);
}