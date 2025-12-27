#include <stdio.h>
#include "hash.h"

int main() {
	char* str;
	printf("Enter a string to hash: ");
	scanf("%s", str);
	str = hash(str);
	printf("\n%s", str);
	/*
	 * This is a hashing function
	 * And all this does is hashes things
	 */
}
