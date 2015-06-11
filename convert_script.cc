#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
using namespace std;

int main()
{
	int R1[] = {1, 3, 5, 7, 9, 11};
	int R2[] = {2, 4, 6, 8, 10, 12};
	int H[] = {-5, -3, 0, 3, 5};

	char file1[1024];
	char file2[1024];
	char command[1024];

	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			if (R2[j] <= R1[i])
				continue;
			for (int k = 0; k < 5; k++)
			{
				for (int l = 0; l < 4; l++)
				{
					sprintf(file1, "testdata/%d_%d_%d_%d.pgm", R1[i], R2[j], H[k], l);
					sprintf(file2, "testdata/%d_%d_%d_%d.jpg", R1[i], R2[j], H[k], l);
					sprintf(command, "convert %s %s", file1, file2);
					system(command);
				}
			}
		}
	}

	return 0;
}
