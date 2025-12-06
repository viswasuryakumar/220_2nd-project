#include <stdio.h>

long factorial(long n)
{
    if (n <= 1)
        return 1;
    return n * factorial(n - 1);
}

int main(void)
{
    long n = 5;
    long result = factorial(n);
    printf("factorial(%ld) = %ld\n", n, result);
    return 0;
}
