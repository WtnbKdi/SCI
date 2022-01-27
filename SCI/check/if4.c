#include <stdio.h>

int main()
{
	int a;
	a = 10;
	if (a != 10) {
		printf("%d", 1);
	}
	else if (a == 9) {
		printf("%d", 2);
	}
	else if (a == 10) {
		printf("%d", 3);
	}
	else {
		printf("%d", 4);
	}
}