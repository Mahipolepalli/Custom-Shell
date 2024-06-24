/********************************************************************************************
This is a template for assignment on writing a custom Shell. 

Students may change the return types and arguments of the functions given in this template,
but do not change the names of these functions.

Though use of any extra functions is not recommended, students may use new functions if they need to, 
but that should not make code unnecessorily complex to read.

Students should keep names of declared variable (and any new functions) self explanatory,
and add proper comments for every logical step.

Students need to be careful while forking a new process (no unnecessory process creations) 
or while inserting the single handler code (should be added at the correct places).

Finally, keep your filename as myshell.c, do not change this name (not even myshell.cpp, 
as you not need to use any features for this assignment that are supported by C++ but not by C).
*********************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>			// exit()
#include <unistd.h>			// fork(), getpid(), exec()
#include <sys/wait.h>		// wait()
#include <signal.h>			// signal()
#include <fcntl.h>			// close(), open()
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

char **parseInput(char *line) //function converts given input line into an array of strings
{
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	int i, tokenIndex = 0, tokenNo = 0;

	for (i = 0; i < strlen(line); i++)
	{

		char readChar = line[i];

		if (readChar == ' ' || readChar == '\n' || readChar == '\t')  //if delimiters are found we consider it as next string in the array of strings
		{
			token[tokenIndex] = '\0';
			if (tokenIndex != 0)
			{
				tokens[tokenNo] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0;
			}
		}
		else
		{
			token[tokenIndex++] = readChar;
		}
	}

	free(token);  //free the token array 
	tokens[tokenNo] = NULL;  //make the last index of array NULL to understand that it is the end of the input
	return tokens;
}

void SIGINThandler(int n){   //this function handles ctrl+C
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	printf("\n%s$", cwd);
    fflush(stdout);
}

void SIGSTPhandler(int n){   //this function handles ctrl+V
    char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	printf("\n%s$", cwd);
    fflush(stdout);
}

int containsOutputRedirection(char **tokens) {   //this function checks whether nput string has output redirection symbol('>')
    int tokenIndex = 0;
    while (tokens[tokenIndex] != NULL) {
        if (strcmp(tokens[tokenIndex], ">") == 0) {
            return tokenIndex; // Found the ">" symbol
        }
        tokenIndex++;
    }
    return -1; // Did not find the ">" symbol
}

int contains1(char **tokens) {
    int tokenIndex = 0;
    while (tokens[tokenIndex] != NULL) {
        if (strcmp(tokens[tokenIndex], "##") == 0) {
            return tokenIndex; // Found the "##" symbol
        }
        tokenIndex++;
    }
    return -1; // Did not find the "##" symbol
}

int contains2(char **tokens) {
    int tokenIndex = 0;
    while (tokens[tokenIndex] != NULL) {
        if (strcmp(tokens[tokenIndex], "&&") == 0) {
            return tokenIndex; // Found the "&&" symbol
        }
        tokenIndex++;
    }
    return -1; // Did not find the "&&" symbol
}

void executeCommand(char **tokens)
{
	// This function will fork a new process to execute a command
	int rc = fork();
		if(rc == 0)  //child process
		{
			if (execvp(tokens[0], tokens) == -1) {    //tokens[0] is the command to be executed and tokens is the array of strings
				printf("Shell: Incorrect command\n");
			}
			exit(0);
		}
		else   //parent process
		{
			wait(NULL);
		}
}

void executeParallelCommands(char **tokens) {
    int numTokens = 0;
    while (tokens[numTokens] != NULL) {
        numTokens++;
    }

    // Initialize an array of pointers to store groups of commands
    int tokenIndex = 0; // Index to iterate through tokens

    // Iterate through the tokens
    while (tokenIndex < numTokens) {
        // Check for the cd command
        if (strcmp(tokens[tokenIndex], "cd") == 0) {
            if (tokenIndex + 1 < numTokens) {
                if (chdir(tokens[tokenIndex + 1]) == 0) {
                    // chdir returns 0 on success
                }
                tokenIndex += 2; // Skip "cd" and its argument
            } 
        } else {
            // Initialize a buffer to store a group of commands
            char **groupBuffer = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
    
            int groupSize = 0; // Size of the current group

            // Continue adding tokens to the current group until "##" is encountered
            while (tokenIndex < numTokens && strcmp(tokens[tokenIndex], "&&") != 0) {
                // Copy the token to the group buffer
                groupBuffer[groupSize] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
                strcpy(groupBuffer[groupSize], tokens[tokenIndex]);
                groupSize += 1;
                tokenIndex++;
            }

            // Null-terminate the group buffer
            groupBuffer[groupSize] = NULL;

            int rc = fork();
            if (rc == 0) {
                // Child process
                if (execvp(groupBuffer[0], groupBuffer) == -1) {
                    printf("Shell: Incorrect command\n");
                    exit(1);
                }
            } else if (rc < 0) {
                printf("Shell: Fork failed\n");
			}

            // Free the memory allocated for the group buffer
            for (int i = 0; i < groupSize; i++) {
                free(groupBuffer[i]);
            }
            free(groupBuffer);

            // Skip past the "##" token (if present)
            if (tokenIndex < numTokens && strcmp(tokens[tokenIndex], "##") == 0) {
                tokenIndex++;
            }
        }
    }
}

void executeSequentialCommands(char **tokens) {
    int numTokens = 0;
    while (tokens[numTokens] != NULL) {
        numTokens++;
    }

    // Initialize an array of pointers to store groups of commands
    int tokenIndex = 0; // Index to iterate through tokens

    // Iterate through the tokens
    while (tokenIndex < numTokens) {
        // Check for the cd command
        if (strcmp(tokens[tokenIndex], "cd") == 0) {
            if (tokenIndex + 1 < numTokens) {
                if (chdir(tokens[tokenIndex + 1]) == 0) {
                    // chdir returns 0 on success
                }
                tokenIndex += 2; // Skip "cd" and its argument
            } 
        } else {
            // Initialize a buffer to store a group of commands
            char **groupBuffer = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
    
            int groupSize = 0; // Size of the current group

            // Continue adding tokens to the current group until "##" is encountered
            while (tokenIndex < numTokens && strcmp(tokens[tokenIndex], "##") != 0) {
                // Copy the token to the group buffer
                groupBuffer[groupSize] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
                strcpy(groupBuffer[groupSize], tokens[tokenIndex]);
                groupSize += 1;
                tokenIndex++;
            }

            // Null-terminate the group buffer
            groupBuffer[groupSize] = NULL;

            int rc = fork();
            if (rc == 0) {
                // Child process
                if (execvp(groupBuffer[0], groupBuffer) == -1) {
                    printf("Shell: Incorrect command\n");
                    exit(1);
                }
            } else if (rc < 0) {
                printf("Shell: Fork failed\n");
            } else {
                // Parent process
                wait(NULL);
            }

            // Free the memory allocated for the group buffer
            for (int i = 0; i < groupSize; i++) {
                free(groupBuffer[i]);
            }
            free(groupBuffer);

            // Skip past the "##" token (if present)
            if (tokenIndex < numTokens && strcmp(tokens[tokenIndex], "##") == 0) {
                tokenIndex++;
            }
        }
    }
}

void executeCommandRedirection(char **tokens, int found)
{
	// This function will run a single command with output redirected to an output file specificed by user
	int outputFileDescriptor = open(tokens[found + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (outputFileDescriptor == -1) {
        printf("Shell: Incorrect command\n");
    }
	int original_stdout = dup(STDOUT_FILENO);
	// Redirect stdout to the output file
    dup2(outputFileDescriptor, STDOUT_FILENO);

    // Close the output file descriptor since it's now a duplicate of STDOUT_FILENO
    close(outputFileDescriptor);

	int rc = fork();

	if(rc == 0){

    // Execute the command with its arguments
		if (found > 0) {
			// Set the last argument in the sequence to NULL
			tokens[found] = NULL;

			// Execute the command using execvp
			if (execvp(tokens[0], tokens) == -1) {
				printf("Shell: Incorrect command\n");
			}
		} 
		exit(0);
	}
	else
	{
		wait(NULL);
		dup2(original_stdout, STDOUT_FILENO);
		close(original_stdout);
	}
}

int main()
{
	// Initial declarations
	char line[MAX_INPUT_SIZE];
	char **tokens;
	int i;
	
	signal(SIGINT, SIGINThandler);
	signal(SIGTSTP, SIGSTPhandler);
	while(1)	// This loop will keep your shell running until user exit command.
	{
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		char cwd[1024];
		getcwd(cwd, sizeof(cwd));
		printf("%s$", cwd);
		scanf("%[^\n]", line);
		getchar();

		// printf("Command entered: '%s' (REMOVE THIS OUTPUT LATER)\n", line);
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; // terminate with new line
		tokens = parseInput(line);

		if (tokens[0] == NULL)	// if empty command, ask for as command again in new line
            continue;

		// Print the prompt in format - currentWorkingDirectory$		
		
		if(strcmp(tokens[0],"exit") == 0)	// When user uses exit command.
		{
			printf("Exiting shell...\n");
			break;
		}

		if(strcmp(tokens[0],"cd") == 0)	// When user uses cd command.
		{
			// Use chdir to change the current working directory
			if(tokens[1] == NULL)
			{
				printf("Shell: Incorrect command\n");
				continue;
			}
			if(tokens[2] == NULL){
				if (chdir(tokens[1]) == 0) {
					// chdir returns 0 on success
				} else {
					// chdir returns -1 on failure
					printf("Shell: Incorrect command\n");
				}
				continue;
			}
		}
		
		int found = containsOutputRedirection(tokens);
		int found1 = contains1(tokens);
		int found2 = contains2(tokens);
		// if(/condition/)
		// 	executeParallelCommands();		// This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
		// else if(/condition/)
		// 	executeSequentialCommands();	// This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
		if(found != -1){
			executeCommandRedirection(tokens, found);	// This function is invoked when user wants redirect output of a single command to and output file specificed by user
			continue;
		}
		else if(tokens[1] == NULL){
			executeCommand(tokens);		// This function is invoked when user wants to run a single commands
			continue;
		}
		else if(found1 != -1)
		{
			executeSequentialCommands(tokens);
			continue;
		}
		else if(found2 != -1)
		{
			executeParallelCommands(tokens);
			continue;
		}
		else
		{
			printf("Shell: Incorrect command\n");
		}
				
	}
	
	return 0;
}