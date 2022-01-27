#include <stdio.h>

int f()
{
	return 100;
}

int main()
{
	int a;
	a = 10;

	if (f()) {
		printf("%d", 1);
	}
}