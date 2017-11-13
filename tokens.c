#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "arraylist.h"

// read_word and read_op loosely adapted from read_number

int
isoperator2(char str) 
{ 
        switch (str) 
        { 
                case '<' : return 1;   
                case '>' : return 1; 
                case '|' : return 1; 
                case '&' : return 1;  
                case ';' : return 1;   
                default: return 0;  
        } 
} 

char*
read_word(const char* line, int i)
{
	int j = 0;
	while (!(isspace(line[i + j])) 
		&& isoperator2(line[i + j]) != 1) {
		j++;
	}

	char* word = malloc(j + 1);
	memcpy(word, line + i, j);
	word[j] = 0;
	return word;
}

char*
read_op(const char* line, int i)
{
	int j = 0;
	while (isoperator2(line[i + j]) == 1) {
		j++;
	}

	char* op = malloc(j + 1);
	memcpy(op, line + i, j);
	op[j] = 0;
	return op;
}
     
arraylist*
tokenize(const char* line)
{
        arraylist* al = create_al();

        int n = strlen(line);
        int i = 0;

	while (i < n) {
		if (isspace(line[i])) {
			i++;
			continue;
		}
		
		if (isoperator2(line[i]) == 1 || isoperator2(line[i]) == 2) {
			char* str = read_op(line, i);
			al_put_in(al, str);
			i += strlen(str);
			free(str);
			continue;
		}

		char* str = read_word(line, i);
		al_put_in(al, str);
		i += strlen(str);
		free(str);
		continue;
	}

	return al;
}					

//int
//main(int _argc, char* _argv[])
//{
//        char line[100];
//
//        while (1) {
//                char* rv = fgets(line, 100, stdin);
//                if (!rv) {
//                        break;
//                }
//
//                arraylist* al = tokenize(line);
//
//               for (int i = 0; i < al->amt; ++i) {
//			printf("%s\n", al->data[i]);
//		} 
//                
//                free_al(al);
//	}
//  
//        return 0;
//}

