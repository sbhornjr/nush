#include <unistd.h> 
#include <stdlib.h> 
#include <string.h> 
#include <stdio.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <assert.h>

#include "arraylist.h" 

int
examine(char* cmd, arraylist* toks);

void
changedirectory(arraylist* toks)
{
	int rv = chdir(al_get(toks, 1));

	if (rv != 0) {
		printf("Error found when attempting to change directory to %s\n", toks->data[1]); 
	}
}

int
isbuiltin(char* cmd)
{
	if (strncmp(cmd, "exit", 4) == 0) {
		return 1;
	}
	else if (strncmp(cmd, "cd", 4) == 0) {
		return 2;
	}
	else {
		return 0;
	}
}

int
execute(char* cmd, arraylist* toks, int back)
{
	int cpid;
	
	if ((cpid = fork())) {
		// parent process

		// Child may still be running until we wait.

		int status; 
		if (back == 0) {
                	waitpid(cpid, &status, 0);
		}
		 
		return WEXITSTATUS(status);
        }
	else {
		// child process 

		// The argv array for the child. 
		// Terminated by a null pointer.
		char* args[toks->amt + 1];

		for (int i = 0; i < toks->amt; ++i) {
			args[i] = al_get(toks, i);
		} 		
		args[toks->amt] = 0;

		execvp(cmd, args); 
		printf("Can't get here, exec only returns on error.");
		_Exit(0); 
        } 
}  

void 
redirin(char* cmd, arraylist* toks, int ii) 
{ 
	FILE* infile = fopen(al_get(toks, ii + 1), "r");
	int sstdin;
	sstdin = dup(0);

	dup2(fileno(infile), 0); 

	close(fileno(infile));        

	arraylist* newal = create_al();
	for (int i = 0; i < ii; ++ i) {
		al_put_in(newal, al_get(toks, i));
	}
	for (int i = ii + 1; i < toks->amt; ++i) {
		al_put_in(newal, al_get(toks, i));
	}

	examine(cmd, newal);

	dup2(sstdin, 1);

	free_al(newal);
} 

void
redirout(char* cmd, arraylist* toks, int ii)
{
	FILE* outfile = fopen(al_get(toks, ii+ 1), "w");
	int sstdout;
	sstdout = dup(1);

	dup2(fileno(outfile), 1);

	close(fileno(outfile));

	arraylist* newal = create_al();
	for (int i = 0; i < ii; ++i) {
		al_put_in(newal, al_get(toks, i));
	}
	
	execute(cmd, newal, 0);

	dup2(sstdout, 1);

	free_al(newal);
}

void
split(char* cmd, arraylist* toks, int ii)
{
	arraylist* newal1 = create_al();

	arraylist* newal2 = create_al();

	for (int i = 0; i < ii; ++i) {
		al_put_in(newal1, al_get(toks, i));
	}

	for (int i = ii + 1; i < toks->amt; ++i) {
		al_put_in(newal2, al_get(toks, i));
	}

 	examine(cmd, newal1);

	examine(cmd, newal2);

	free_al(newal1);
	free_al(newal2);
}

void
andand(char* cmd, arraylist* toks, int ii)
{
	arraylist* left = create_al();
	arraylist* right = create_al();

	for (int i = 0; i < ii; ++i) {
		al_put_in(left, al_get(toks, i));
	}
	for (int i = ii + 1; i < toks->amt; ++i) {
		al_put_in(right, al_get(toks, i));
	}

	int leftnum = examine(cmd, left);
	
	if (leftnum == 0) {
		examine(al_get(right, 0), right);
	}

	free_al(left);
	free_al(right);	
}	

void 
oror(char* cmd, arraylist* toks, int ii)
{
        arraylist* left = create_al(); 
        arraylist* right = create_al(); 
 
        for (int i = 0; i < ii; ++i) { 
                al_put_in(left, al_get(toks, i)); 
        } 
        for (int i = ii + 1; i < toks->amt; ++i) { 
                al_put_in(right, al_get(toks, i)); 
        }

        int leftnum = examine(cmd, left); 
         
        if (leftnum == 1) { 
                examine(al_get(right, 0), right);  
        } 
    
        free_al(left);  
        free_al(right);
}

void
backgrnd(char* cmd, arraylist* toks, int ii)
{
	arraylist* newal = create_al();
	for (int i = 0; i < ii; ++i) {
		al_put_in(newal, al_get(toks, i));
	}

	execute(cmd, newal, 1);
	
	free_al(newal);
}

void
mypipe(char* cmd, arraylist* toks, int ii)
{
	arraylist* left = create_al();  
	arraylist* right = create_al();  
  
	for (int i = 0; i < ii; ++i) {  
		al_put_in(left, al_get(toks, i));  
        }  
	for (int i = ii + 1; i < toks->amt; ++i) {  
	al_put_in(right, al_get(toks, i));  
	}

	int cpid;
	if ((cpid = fork())) {
		// parent
		int status;
		waitpid(cpid, &status, 0);
	}
	else {
		// child
		int pipe_fds[2];
		int rv = pipe(pipe_fds);
		if (rv == -1) {
			perror("fail on pipe");
		}

		int p_read = pipe_fds[0];
		int p_write = pipe_fds[1];
		int cpid2;

		if ((cpid2 = fork())) {
			// parent
			close(0);
			dup(p_read);
			close(p_write);
			
			int status2;
			waitpid(cpid2, &status2, 0);

			char* args[right->amt]; 
			for (int i = 0; i < right->amt; ++i) {
				args[i] = al_get(right, i);
			}
			args[right->amt] = 0;
			
			examine(al_get(right, 0), right);
			_Exit(0);
		}
		else {
			// child
			close(1);
			dup(p_write);
			close(p_read);
			
			char* args[left->amt];
			for (int i = 0; i < left->amt; ++i) {
				args[i] = al_get(left, i);
			}
			args[left->amt] = 0;	
			
			execvp(al_get(left, 0), args);
			_Exit(0);
		}
	}

	free_al(left);
	free_al(right);
}

int
examine(char* cmd, arraylist* toks)
{ 
	int cont1 = al_contains(toks, ";");
	int cont2 = al_contains(toks, "<");
	int cont3 = al_contains(toks, ">"); 
	int cont4 = al_contains(toks, "&&");
	int cont5 = al_contains(toks, "||");
	int cont6 = al_contains(toks, "&");
	int cont7 = al_contains(toks, "|");

	if (cont1 != -1) {
		split(cmd, toks, cont1);
	}
	else if (cont2 != -1) {
		redirin(cmd, toks, cont2);
	}
	else if (cont3 != -1) {
		redirout(cmd, toks, cont3);
	}
	else if (cont4 != -1) {
		andand(cmd, toks, cont4);
	}
	else if (cont5 != -1) {
		oror(cmd, toks, cont5);
	}
	else if (cont6 != -1) {
		backgrnd(cmd, toks, cont6);
	}
	else if (cont7 != -1) {
		mypipe(cmd, toks, cont7);
	}
	else {  
		int rv = execute(cmd, toks, 0);
		return rv;
	}
}

int
main (int argc, char* argv[])
{
	FILE* script;
	if (argc > 1) {                        
		script = fopen(argv[1], "r");
	}

	while (1) {
		char* cmdline[256];
		char* rv;
		
		if (argc == 1) {
			printf("nush$ ");
			fflush(stdout);
			rv = fgets(cmdline, 256, stdin);
		}

		if (!rv) {
			break;
		}

		arraylist* toks = tokenize(cmdline);

		if (toks->amt == 0) {
			continue;
		}

		char* cmd = al_get(toks, 0);

		if (isbuiltin(cmd) == 1) {
			break;
		}
		else if (isbuiltin(cmd) == 2) {
			changedirectory(toks);
			continue;
		}

		examine(cmd, toks);		

		free_al(toks);
	}

	return 0;
}
