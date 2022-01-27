#include <stdio.h>

int main()
{
	int a;
	a = 0;
	while (a < 10) {
		a++;
		if (a == 5) {
			break;
		}

		printf("%d", a);
	}
}