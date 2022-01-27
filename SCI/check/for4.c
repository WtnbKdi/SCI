int a;

int f()
{
	++a;
	return 0;
}

int main()
{
	for (a = 0; a < 10; f()) {
		printf("%d", a);
	}
}