#include <stdio.h>

int fib1(int num)
{
	int a = 0, b = 1, i;
	for (i = 0; i < num; i++) {
		a = b - a;
		b = a + b;
	}
	return b - a;
}

int fib2(int num)
{
	if (num == 0 || num == 1) {
		return 1;
	}
	return fib2(num - 1) + fib2(num - 2);
}

int main()
{
	int i;
	for (i = 0; i < 10; i++) {
		printf("%d", fib2(i));
	}
}