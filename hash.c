/* hash.c */
#ifndef HASH_C
#define HASH_C

#include "hash.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/*
 * Returns the number of decimal digits in a positive long long.
 */
static long long digit_return(long long num)
{
	long long digits = 0;
	if (num == 0) return 1;
	while (num > 0) {
		num /= 10;
		digits++;
	}
	return digits;
}

/*
 * Compute 10^n (n ≥ 0) using integer arithmetic.
 */
static long long pow10ll(int n)
{
	long long r = 1;
	while (n-- > 0)
		r *= 10;
	return r;
}

/*
 * Integer square root using Newton’s method.
 * Returns floor(sqrt(n)).
 */
static long long isqrt(long long n)
{
	long long x = n;
	long long y = (x + 1) / 2;
	while (y < x) {
		x = y;
		y = (x + n / x) / 2;
	}
	return x;
}

/*
 * Given coefficients of ax² + bx + c, mashes the two roots
 * (real or complex) into a single positive long long.
 * For complex roots the real and imaginary parts are packed.
 * For real roots they are packed low-value-first.
 * The result is at most 12 digits, fitting safely in 64 bits.
 */
static long long quadratic_roots(long long a, long long b, long long c)
{
	long long discriminant = b * b - 4 * a * c;
	long long base = (-b) / (2 * a);
	long long imag_part = 0, real_part = 0;

	if (discriminant < 0) {
		discriminant = -discriminant;
		imag_part = isqrt(discriminant);
		real_part = base;
	} else {
		long long sqrt_d = isqrt(discriminant);
		long long root1 = base + sqrt_d / (2 * a);
		long long root2 = base - sqrt_d / (2 * a);
		real_part = root1 < root2 ? root1 : root2;
		imag_part = root1 < root2 ? root2 : root1;
	}

	if (real_part < 0)  real_part = -real_part;
	if (imag_part < 0)  imag_part = -imag_part;

	/* pack as RRRRRR III (where each up to six digits) */
	return real_part * 1000000LL + imag_part;
}

/*
 * Split an arbitrary-length decimal number (given as value + digit
 * count) into three chunks and feed them to quadratic_roots().
 */
static long long quadratic_division(long long num, int digits)
{
	int div = digits / 3;
	int rem = digits - 2 * div;  /* first chunk may be bigger */

	long long p1 = pow10ll(div);
	long long p2 = pow10ll(div + rem);

	long long constant  = num % p1;
	long long x         = (num / p1) % p1;
	long long x_square  = num / p2;

	return quadratic_roots(x_square, x, constant);
}
/*
 * Irreversible hash of a printable C-string.
 * Returns a 16-hex-digit NUL-terminated string.
 * Safe to call repeatedly; the buffer is static (not thread-safe).
 */
char *hash(const char *str)
{
	static char out[17];   /* 16 hex digits + NUL */

	/* 1. Collapse the string into a big base-10 integer. */
	long long num = 0;
	for (size_t i = 0; str[i] != '\0'; i++) {
		uint8_t ch = (uint8_t)str[i];

		/* Expand num so that digits never collide. */
		if      (ch < 10)   num *= 10;
		else if (ch < 100)  num *= 100;
		else                num *= 1000;

		num += ch;
	}

	/* 2. Derive a second 64-bit value via the quadratic gadget. */
	long long digits = digit_return(num);
	long long quad   = quadratic_division(num, (int)digits);

	/* 3. Mix the two with a variant of MurmurHash3’s finalizer.    */
	uint64_t h = (uint64_t)(num ^ (quad * 0x9E3779B97F4A7C15ULL)); /* φ-based salt */
	h ^= h >> 33;
	h *= 0xFF51AFD7ED558CCDULL;
	h ^= h >> 33;
	h *= 0xC4CEB9FE1A85EC53ULL;
	h ^= h >> 33;

	/* 4. Emit as fixed-width lowercase hexadecimal.*/
	snprintf(out, sizeof(out), "%016llx", (unsigned long long)h);
	return out;
}

#endif /* HASH_C */
