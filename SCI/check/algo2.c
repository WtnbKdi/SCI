#include <stdio.h>

int main()
{
	int a, b;
	for (a = 0; a < 10; a++) {
		for (b = 0; b < 10; b++) {
			printf("%d", a * b);
		}
	}
}