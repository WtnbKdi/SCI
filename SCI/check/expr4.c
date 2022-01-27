#include <stdio.h>

int f()
{
	return 100;
}

int main()
{
	int a;
	a = 5 * f();
	printf("%d", a);
}