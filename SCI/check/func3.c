#include <stdio.h>

int f(int a, int b)
{
	if (100 < (a * b)) {
		return 1;
	}
	else {
		return 0;
	}
}

int main()
{
	printf("%d", f(2, 100));
}