#include <stdio.h>

int f(int a, int b)
{
	return a * b;
}

int main()
{
	int a, b;
	a = 3;
	b = 5;
	printf("%d", f(a, b));
}