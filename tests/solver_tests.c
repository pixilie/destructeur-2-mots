#include <stdio.h>
#include <stdlib.h>
#include "../include/test_helpers.h"
#include "../include/solver.h"

int solve_word(char *word, int x1_out, int y1_out, int x2_out, int y2_out)
{
	int x1, y1, x2, y2;
	solve("tests/solver_grid_sample.txt", word, &x1, &y1, &x2, &y2);
	if(x1 != x1_out || y1 != y1_out || x2 != x2_out || y2 != y2_out)
	{
		print_fail();
		printf("Failed to solve word %s : expected (%i, %i)(%i, %i), got: (%i, %i)(%i, %i)\n", 
			word, x1_out, y1_out, x2_out, y2_out, x1, y1, x2, y2);
		return EXIT_FAILURE;
	}
	else
	{
		print_success();
		printf("Solved word %s correctly, got: ", word);
		if(x1 == -1)
		{
			printf("Not found\n");
		}
		else
		{
			printf("(%i, %i)(%i, %i)\n", x1, y1, x2, y2);
		}
		return EXIT_SUCCESS;
	}
}
	
int test_horizontal()
{
    printf("\n--- Horizontal Tests ---\n");
    int res = solve_word("horizontal", 0, 0, 9, 0); 
    res &= solve_word("HORIZONTAL", 0, 0, 9, 0);
    res &= solve_word("LATNOZIROH", 9, 0, 0, 0);
    res &= solve_word("hori", 0, 0, 3, 0);
    res &= solve_word("HELLO", 9, 8, 5, 8);
    res &= solve_word("world", 5, 5, 1, 5);
    res &= solve_word("toit", 9, 5, 6, 5);
    res &= solve_word("GOD", 3, 4, 5, 4); 
    res &= solve_word("EPITA", -1, -1, -1, -1); //Not Found 
    if (res == 1)
    {
        printf("All tests passed.\n");
	return EXIT_SUCCESS;
    }
    else
    {
        printf("Some tests did not pass.\n");
	return EXIT_FAILURE;
    }
    return res;
}

int test_vertical()
{
    printf("\n--- Vertical Tests ---\n");
    int res = solve_word("vertical", 9, 7, 9, 0); 
    res &= solve_word("VERTICAL", 9, 7, 9, 0); 
    res &= solve_word("LACITREV", 9, 0, 9, 7); 
    res &= solve_word("khld", 1, 9, 1, 6);
    res &= solve_word("vetcal", -1, -1, -1, -1); //Not Found 
    if (res == 1)
    {
        printf("All tests passed.\n");
	return EXIT_SUCCESS;
    }
    else
    {
        printf("Some tests did not pass.\n");
	return EXIT_FAILURE;
    }
    return res;
}

int test_diagonal()
{
    printf("\n--- Diagonal Tests ---\n");
    int res = solve_word("diagonal", 0, 1, 7, 8); 
    res &= solve_word("DIAGONAL", 0, 1, 7, 8); 
    res &= solve_word("find", 4, 8, 1, 5); 
    res &= solve_word("GOLDORAK", 8, 1, 1, 8); 
    res &= solve_word("diagal", -1, -1, -1, -1); //Not Found 
    if (res == 1)
    {
        printf("All tests passed.\n");
	return EXIT_SUCCESS;
    }
    else
    {
        printf("Some tests did not pass.\n");
	return EXIT_FAILURE;
    }
    return res;
}
int main()
{
    printf("\n--- Solver tests ---\n");
    int passed = test_horizontal() & test_vertical() & test_diagonal();
    return passed ? 0 : 1;
}
