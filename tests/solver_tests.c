#include "../include/test_helpers.h"
#include "../include/solver.h"

#include <stdio.h>
#include <stdlib.h>

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
    print_test_subcategory("Horizontal Tests");
    
    int res = solve_word("horizontal", 0, 0, 9, 0); 
    res &= solve_word("HORIZONTAL", 0, 0, 9, 0);
    res &= solve_word("LATNOZIROH", 9, 0, 0, 0);
    res &= solve_word("hori", 0, 0, 3, 0);
    res &= solve_word("HELLO", 9, 8, 5, 8);
    res &= solve_word("world", 5, 5, 1, 5);
    res &= solve_word("toit", 9, 4, 6, 4);
    res &= solve_word("GOD", 3, 4, 5, 4); 
    res &= solve_word("EPITA", -1, -1, -1, -1); //Not Found
    
    if (res == 1)
    {
        print_success();
        printf("Correctly solved all horizontal words\n");
    }
    else
    {
        print_fail();
        printf("Some horizontal words were not solved correctly\n");
    }
    return res;
}

int test_vertical()
{
    print_test_subcategory("Vertical Tests");
    
    int res = solve_word("vertical", 9, 7, 9, 0); 
    res &= solve_word("VERTICAL", 9, 7, 9, 0); 
    res &= solve_word("LACITREV", 9, 0, 9, 7); 
    res &= solve_word("khld", 1, 8, 1, 5);
    res &= solve_word("vetcal", -1, -1, -1, -1); //Not Found
    
    if (res == 1)
    {
        print_success();
        printf("Correctly solved all vertical words\n");
    }
    else
    {
        print_fail();
        printf("Some vertical words were not solved correctly\n");
    }
    return res;
}

int test_diagonal()
{
    print_test_subcategory("Diagonal Tests");
    
    int res = solve_word("diagonal", 0, 1, 7, 8); 
    res &= solve_word("DIAGONAL", 0, 1, 7, 8); 
    res &= solve_word("find", 4, 8, 1, 5); 
    res &= solve_word("GOLDORAK", 8, 1, 1, 8); 
    res &= solve_word("diagal", -1, -1, -1, -1); //Not Found
    
    if (res == 1)
    {
        print_success();
        printf("Correctly solved all diagonal words\n");
    }
    else
    {
        print_fail();
        printf("Some diagonal words were not saved correctly\n");
    }
    return res;
}

int main()
{
    print_test_category("Solver Tests");
    
    int passed = 1;
    if (!test_horizontal())
    {
        passed = 0;
    }
    
    if (!test_vertical())
    {
        passed = 0;
    }
    
    if (!test_diagonal())
    {
        passed = 0;
    }

    if (passed)
    {
        print_all_tests_passed("Solver Tests");
    }
    else
    {
        print_some_tests_failed("Solver Tests");
    }
    
    return passed ? 0 : 1;
}
