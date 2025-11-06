#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Terminal color codes for pretty output
#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"

void print_fail()
{
	printf(COLOR_RED "[FAIL] " COLOR_RESET);
}

void print_success()
{
	printf(COLOR_GREEN "[SUCCESS] " COLOR_RESET);
}
