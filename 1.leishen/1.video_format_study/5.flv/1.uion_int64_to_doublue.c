#include <stdio.h>
#include <stdint.h>
int main()
{
	union DOUBLE
	{
		double  number;
		int64_t data;
	};
	union DOUBLE num;
	num.data = 0x4060A8F5C28F5C29;
	printf("%lf\n",num.number);
	getchar();
	return 0;
}
