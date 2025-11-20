#include <stdio.h>
#include <string.h>

// Terminal color codes for pretty output
#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"

void print_fail()
{
	printf(COLOR_RED "[FAIL] " COLOR_RESET);
}

void print_success()
{
	printf(COLOR_GREEN "[SUCCESS] " COLOR_RESET);
}

void print_all_tests_passed(const char *test_name)
{
	printf("\n");
	print_success();
	printf("%s : All tests passed\n", test_name);
}

void print_some_tests_failed(const char *test_name)
{
	printf("\n");
	print_fail();
	printf("%s : Some tests did not pass\n", test_name);
}

void print_test_category(const char *test_category)
{
	printf("\n");
	printf(COLOR_BLUE "----- ");
	printf("%s", test_category);
	printf(" -----\n" COLOR_RESET);
}

void print_test_subcategory(const char *test_subcategory)
{
	printf("\n");
	printf(COLOR_YELLOW "--- ");
	printf("%s", test_subcategory);
	printf(" ---\n" COLOR_RESET);
	printf("\n");
}
