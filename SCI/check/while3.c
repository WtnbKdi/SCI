#include <stdio.h>

int main()
{
	int a, b;
	a = 0;
	while ((a = ++a) < 10) {
		printf("%d", a);
	}
}