/* Reference C Implementation of Recursive Factorial
 *
 * This demonstrates the recursive factorial function that we'll
 * implement in SimpleCPU16 assembly language.
 */

#include <stdio.h>

/* Recursive factorial function
 * factorial(n) = n * factorial(n-1), with base case factorial(0) = 1
 */
int factorial(int n) {
    if (n <= 1) {
        return 1;  // Base case
    } else {
        return n * factorial(n - 1);  // Recursive case
    }
}

/* Main program - driver code */
int main() {
    int n = 5;
    int result;

    printf("Computing factorial of %d\n", n);
    result = factorial(n);
    printf("Result: %d! = %d\n", n, result);

    return 0;
}

/* Expected output:
 * Computing factorial of 5
 * Result: 5! = 120
 *
 * Call trace:
 * main() calls factorial(5)
 *   factorial(5) calls factorial(4)
 *     factorial(4) calls factorial(3)
 *       factorial(3) calls factorial(2)
 *         factorial(2) calls factorial(1)
 *           factorial(1) returns 1
 *         factorial(2) returns 2 * 1 = 2
 *       factorial(3) returns 3 * 2 = 6
 *     factorial(4) returns 4 * 6 = 24
 *   factorial(5) returns 5 * 24 = 120
 * main() receives 120
 */