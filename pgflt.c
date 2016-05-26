#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	double* pBigArray = (double*)malloc(sizeof(double) * 536870912*4); 
	   int iIndex = rand() % 536870912*4;
	   printf("data: %lf\n", pBigArray[iIndex]);

	return 0;
}

