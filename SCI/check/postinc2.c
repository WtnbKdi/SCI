#include <stdio.h>

int main() 
{
    int a, b;
    a = 0;
    b = a++ + a++;
	printf("%d", b);
}