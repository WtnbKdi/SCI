#include <stdio.h>

int main()
{
	int a;
	for (a = 0; a < 5; a++) {
		if (a == 2) {
			break;
		}

		printf("%d", a);
	}
}