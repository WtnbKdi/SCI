#include <stdio.h>

int main()
{
	int a;
	a = 0;

	do {
		++a;

		if (a != 10) {
			printf("%d", 0);
		}
		else {
			printf("%d", 1);
		}

	} while (a < 10);
}