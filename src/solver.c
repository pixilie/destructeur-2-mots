#include <stdio.h>
#include <stdlib.h>



void solve(char filename[], char word[],int* x1, int* y1, int* x2, int* y2)
{
	int nbcolls = 0;
	int nblines = 0;

	int tempx1;
	int tempy1;
	int tempx2;
	int tempy2;
	
	FILE *f = fopen(filename, "r");
	
	if (f == NULL)
	{
		perror("Can't open the file");
    		exit(EXIT_FAILURE);
	}

	char c = fgetc(f);
	while(c != '\r' && c != '\n' && c != '\0')
	{
		c = fgetc(f);
		nbcolls ++;
	}
	rewind(f);
	
	char useless_line[256];
	while (fgets(useless_line, sizeof(useless_line), f) != NULL)
		nblines ++;
	rewind(f);
	
	char tab[nblines][nbcolls];
	char line[nbcolls + 2];
	int i = 0;
	int j = 0;
	
	while(fgets(line, sizeof(line), f) != NULL)
	{
		while(line[i] != '\r' && line[i] != '\n' && line[i] != '\0')
		{
			tab[j][i] = line[i];
			i ++;
		}
		j ++;
		i = 0;
	}
	fclose(f);

}



int main(int argc, char* argv[])
{
	/*
	call type : ./solver filename word

	file type : at least 5 rows and 5 colls of upper letter (square)
	HORIZONTAL
	DXRAHCLBGA
	DIKCILEOKC
	IGAJHYLYHI
	HGFGODTIOT
	GDLROWKBFR
	PLNRDNERGE
	JHAIDUAJGV
	UKGFFOLLEH

	output type : (x1, y1)(x2, y2) -> coo of the first and last letter of the word in the grid
	*/
	
	if(argc < 3)
	{
		perror("Not enough arguments");
    		exit(EXIT_FAILURE);
	}

	if(argc > 3)
	{
		perror("Too many arguments");
    		exit(EXIT_FAILURE);
	}

	int* x1 = malloc(sizeof(int));
	int* y1 = malloc(sizeof(int));
	int* x2 = malloc(sizeof(int));
	int* y2 = malloc(sizeof(int));

	solve(argv[1], argv[2], x1, y1, x2, y2);
	
	*x1 = *y1 = *x2 = *y2 = 0; // to remove

	printf("( %i %i )( %i %i )", *x1, *y1, *x2, *y2);
	
	free(x1);
	free(y1);
	free(x2);
	free(y2);

	return 0;
}
