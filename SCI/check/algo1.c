// �o�u���\�[�g

#include <stdio.h>

int ary[10];

int main()
{
	int save;
	int i, j, size;
	size = 10;
	for (i = 0; i < size; i++) {
		ary[i] = i;
	}

	for (i = 0; i < size; i++) {
		for (j = 0; j < size - 1; j++) {
			if (ary[j] < ary[j + 1]) {
				save = ary[j];
				ary[j] = ary[j + 1];
				ary[j + 1] = save;
			}
		}
	}

	for (i = 0; i < size; i++) {
		printf("%d", ary[i]);
	}
}