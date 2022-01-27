#include <stdio.h>

int main()
{
	int a, b;
	a = 0;
	while (1) {
		if (a == 10) {
			break;
		}
		printf("%d", a);
		a++;
	}
}