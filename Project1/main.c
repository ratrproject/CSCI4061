#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>

#include "util.h"

#define DEBUG 1

struct node{
	char * name;
	struct node *parent[MAX_NODES];
	struct node *child[MAX_CHILDREN];
	int boolRun; //Check if a process has been run yet.
};

struct node graphRoot();
target_t targetList[MAX_NODES]; //Array of target structs
int nTargetCount=-1; //Counter for placing targets in array

//This function will parse makefile input from user or default makeFile. 
int parse(char * lpszFileName)
{
	int nLine = 0;
	char szLine[1024];
	char * lpszLine;
	FILE * fp = file_open(lpszFileName);
	
	int nDependencyCount=0; //Counter for placing dependencies in array inside target
	
	int boolHasCommand = 0; //Boolean for checking if we already have a command

	if(fp == NULL)
	{
		return -1;
	}

	while(file_getline(szLine, fp) != NULL) 
	{
		nLine++;

		//Remove newline character at end if there is one
		lpszLine = strtok(szLine, "\n");

		if (lpszLine != NULL) //Checks if "blank line"
		{
			if (strncmp(lpszLine, "\t", 1) != 0) //If there is no leading tab
			{
				if (strncmp(lpszLine, "#", 1) != 0) //If the line is not a comment
				{
					int boolHasColon = 0;
					while (strlen(lpszLine) > 0) //Check for any illegal characters in line
					{
						if (isalnum(*lpszLine) || isblank(*lpszLine) || *lpszLine == '.')
						{
							lpszLine++;
						}
						else if (*lpszLine == ':' && !boolHasColon)
						{
							boolHasColon = 1;
							lpszLine++;
						}
						else
						{
							printf("ERROR: Illegal character on Line %d\n", nLine);
							return -1;
						}
					}
					
					lpszLine = szLine;
					while (*lpszLine == ' ') {lpszLine++;} //Skip all leading spaces
					if (strchr(lpszLine, ':') == NULL) //If there is no colon in line
					{
						printf("ERROR: No target on Line %d\n", nLine);
						return -1;
					}
					
					nTargetCount++;
					nDependencyCount = 0;
					boolHasCommand = 0;

					lpszLine = strtok(lpszLine, ":");
					if (strchr(lpszLine, ' ')) //If there is a space in the target name
					{
						printf("ERROR: Illegal target name on Line %d\n", nLine);
						return -1;
					}
					int i;
					for (i = 0; i < nTargetCount; i++) //Check for duplicate target names
					{
						if (strcmp(lpszLine, targetList[i].szTarget) == 0)
						{
							printf("ERROR: Duplicate target on Line %d\n", nLine);
							return -1;
						}
					}
					strcpy(targetList[nTargetCount].szTarget, lpszLine);
					
					lpszLine = strtok(NULL, " \t");
					while (lpszLine != NULL)
					{
						strcpy(targetList[nTargetCount].szDependencies[nDependencyCount], lpszLine);
						nDependencyCount++;
						lpszLine = strtok(NULL, " \t");
					}
					targetList[nTargetCount].nDependencyCount = nDependencyCount;
					nDependencyCount = 0;
				}
			}
			else //If there is a leading tab
			{
				while (strlen(lpszLine) > 0) //Check for any illegal characters in line
				{
					if (isalnum(*lpszLine) || isblank(*lpszLine) || strchr("-.'\"", *lpszLine))
					{
						lpszLine++;
					}
					else
					{
						printf("ERROR: Illegal character on Line %d\n", nLine);
						return -1;
					}
				}

				lpszLine = szLine;
				while (isspace(*lpszLine)) {lpszLine++;} //Skip all leading whitespace
				if (strlen(lpszLine) > 0 && strncmp(lpszLine, "#", 1) != 0) //If the line is not empty or a comment
				{
					if (boolHasCommand) //If we already have a command
					{
						printf("ERROR: Second command on Line %d\n", nLine);
						return -1;
					}
					else //If we don't have a command
					{
						strcpy(targetList[nTargetCount].szCommand, lpszLine);
						boolHasCommand = 1;
					}
				}
			}
		}
	}
	
	//DEBUG CODE: Prints the target information to terminal
	if (DEBUG)
	{
		int i = 0;
		while (nTargetCount >= i)
		{
			printf("%d: %s\n", i+1, targetList[i].szTarget);
			nDependencyCount = targetList[i].nDependencyCount;
			int j = 0;
			while (nDependencyCount > j) {
				printf("\tDependency %d: %s\n", j+1, targetList[i].szDependencies[j]);
				j++;
			}
			printf("\tCommand: %s\n", targetList[i].szCommand);
			i++;
		}
	}

	//Close the makefile. 
	fclose(fp);

/*
	//Building graph here, to use all these instance vars.
	struct node tempnodes[nTargetCount];
	int i=0;
	while(i<nTargetCount){
		tempnodes[i]=new struct node();
		tempnodes[i].name= targetList[i].szCommand;
		i++;
	}
	int temp =0;
	int temp1 =0;
	struct node tempnod = NULL;
	char * tempstr = NULL;
	i =0;
	while(i<nTargetCount){
		if(targetList[i].nDependencyCount == 0){
			graphRoot.child[temp]= *tempnodes[i];
			temp++;
		}
		i++;
	}
	temp=0;
	i =0;
	while(i< nTargetCount){
		tempnod=targetList[i];
		temp=tempnod.nDependencyCount;
		int j=0;
		while(j<temp){
			tempstr= tempnod.szDependencies[j];
			int k=0;
			while(k< nTargetCount){
				if(tempstr==targetList[k].szCommand){
					int l=0;
					while(tempnodes[k].child[l]!=null) l++;
					tempnodes[k].child[l]= *tempnod;
					l=0;
					while(tempnod.parent[l]!=null) l++;
					tempnod.parent[l]= *tempnodes[k];
					k= nTargetCount;
				}
				k++;
			}
			j++;
		}
		i++;
	}
*/

	//delete temp, temp1, tempnod, tempstr; //may also delete tempnodes pointers, since nodes are now linked via graphRoot.

	return 0;
}

void show_error_message(char * lpszFileName)
{
	fprintf(stderr, "Usage: %s [options] [target] : only single target is allowed.\n", lpszFileName);
	fprintf(stderr, "-f FILE\t\tRead FILE as a maumfile.\n");
	fprintf(stderr, "-h\t\tPrint this message and exit.\n");
	fprintf(stderr, "-n\t\tDon't actually execute commands, just print them.\n");
	fprintf(stderr, "-B\t\tDon't check files timestamps.\n");
	fprintf(stderr, "-m FILE\t\tRedirect the output to the file specified .\n");
	exit(0);
}


int main(int argc, char **argv) 
{
	// Declarations for getopt
	extern int optind;
	extern char * optarg;
	int ch;
	char * format = "f:hnBm:";
	
	// Default makefile name will be Makefile
	char szMakefile[64] = "./testcases/make_testcase/Makefile"; // default is Makefile
	char szTarget[64];
	char szLog[64];

	while((ch = getopt(argc, argv, format)) != -1) 
	{
		switch(ch) 
		{
			case 'f':
				strcpy(szMakefile, strdup(optarg));
				break;
			case 'n':
				break;
			case 'B':
				break;
			case 'm':
				strcpy(szLog, strdup(optarg));
				break;
			case 'h':
			default:
				show_error_message(argv[0]);
				exit(1);
		}
	}

	argc -= optind;
	argv += optind;

	// at this point, what is left in argv is the targets that were 
	// specified on the command line. argc has the number of them.
	// If getopt is still really confusing,
	// try printing out what's in argv right here, then just running 
	// with various command-line arguments.

	if(argc > 1)
	{
		show_error_message(argv[0]);
		return EXIT_FAILURE;
	}

	//You may start your program by setting the target that make4061 should build.
	//if target is not set, set it to default (first target from makefile)
	if(argc == 1)
	{
	}
	else
	{
	}


	/* Parse graph file or die */
	if((parse(szMakefile)) == -1) 
	{
		return EXIT_FAILURE;
	}

	//after parsing the file, you'll want to check all dependencies (whether they are available targets or files)
	//then execute all of the targets that were specified on the command line, along with their dependencies, etc.
	return EXIT_SUCCESS;
}


