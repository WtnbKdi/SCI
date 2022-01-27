#include <stdio.h>

int f(int _num)
{
	if (_num == 0) {
		return 1;
	}

	return f(_num - 1) * _num;
}

int main()
{
	printf("%d", f(3));
}