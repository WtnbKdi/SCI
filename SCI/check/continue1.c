#include <stdio.h>

int main()
{
	int a;
	a = 0;
	while (1) {
		a++;
		if (a < 10) {
			continue;
		}
		else {
			printf("%d", a);
			break;
		}
	}
}