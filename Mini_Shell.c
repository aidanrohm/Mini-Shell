// Aidan Rohm ---------- CISC5590 ---------- Professor Dent ---------- Final Project - My Own Shell ---------- DUE: 12/10/24
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>

// Important preliminaries for later code
#define MAX_INPUT_SIZE 1024                                     // Important to set the input size
#define MAX_TOKENS 100                                          // Important to set the length of the input array
#define HISTORY_SIZE 20                                         // Length of the history queue (NOTE: no duplicates)
#define COLOR_BLUE "\033[1;34m"                                 // Used to print directories in blue (when using ls command) - for user experience
#define COLOR_RESET "\033[0m"                                 

// Used for the history queue and command
char *history[HISTORY_SIZE];
int history_count = 0;

// <----------------------------------------------------------------------> FUNCTION PROTOTYPES <------------------------------------------------------------------->
void TrimInput(char* input);
void ParseUserInput(char* input, char** tokens);
void AddToHistory(const char *command);
void PrintHistory();
void ExecuteCommandFromHistory(int index);
void HandleRedirection(char **tokens);
int CommandRequiresArguments(char* command);
char *CommandGenerator(const char *text, int state);
char **CustomCompletion(const char *text, int start, int end);
void ExecuteLS();

// <----------------------------------------------------------------------------> MAIN <---------------------------------------------------------------------------->
int main() {
    
    // Setting the custom tab-completion function
    rl_attempted_completion_function = CustomCompletion;

    // Creating input and token structures
    char input[MAX_INPUT_SIZE];
    char* tokens[MAX_TOKENS];

    // Creating a temporary directory to act as the isolated environment - or checking if it already exists
    const char* temp_directory = "/tmp/shell_env";
    struct stat st;

    if (stat(temp_directory, &st) == -1) {
        if (mkdir(temp_directory, 0777) == -1) {
            perror("Failed to create the temporary directory/isolated environment . . . ");         // Error case
            exit(1);
        }
        printf("Isolated environment created: %s\n", temp_directory);
    } else {
        printf("Temporary directory/isolated environment already exists: %s\n", temp_directory);
    }

    // Change to the new directory (isolated environment)
    if(chdir(temp_directory) == -1) {
        perror("Failed to change to the temporary directory/isolated environment . . . ");          // Error case
        exit(1);
    }

    // Prompt after changing to the isolated environment
    printf("Welcome to Aidan's Shell!\n");
    
    while (1) {
        // Get the current working directory (cwd) for the prompt
        char cwd[MAX_INPUT_SIZE];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            strcat(cwd, "/- <<>> ");
        } else {
            perror("getcwd failed");                                                                // Error case
            strcpy(cwd, "?/- <<>> ");
        }
        // Prompt user for their input
        char* input = readline(cwd);
        
        if(!input) {
            printf("\nExiting the shell . . .\n");
            break;
        }

        // Trimming the user's input of any unwanted characters
        TrimInput(input);
        if(strlen(input) == 0) {
            free(input);    
            continue;
        }

        // Adding the user's input command to the history queue
        AddToHistory(input);

        // Parsing the user's input into tokens 
        ParseUserInput(input, tokens);
        
        // Exit the shell if the user enters "exit"
        if(tokens[0] != NULL && strcmp(tokens[0], "exit") == 0) {
            break;
        }
        
        // If the input is history, display the queue, then if the user inputs the history operator, be ready to execute that command
        if (strcmp(tokens[0], "history") == 0) {
            PrintHistory();
            continue;
        } else if (input[0] == '!' && strlen(input) > 1) {
            int index = atoi(input + 1) - 1;
            ExecuteCommandFromHistory(index);
            continue;
        }

        // Handle some specific commands and some errors that may occur with them

        // Handling cd
        if (strcmp(tokens[0], "cd") == 0) {
            if (tokens[1] == NULL) {
                fprintf(stderr, "cd: expected argument to \"cd\"\n");                                       
            } else {
                if (chdir(tokens[1]) == -1) {
                    perror("cd");
                }
            }
            continue;                                           // Skip the rest of the loop after handling 'cd'
        }

        // To handle the pwd command
        if (strcmp(tokens[0], "pwd") == 0) {
            char cwd[MAX_INPUT_SIZE];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("%s\n", cwd);
            } else {
                perror("pwd");
            }
            continue;                                           // Skip the rest of the loop after handling 'pwd'
        }

        // Handling the ls command 
        if (strcmp(tokens[0], "ls") == 0) {
            ExecuteLS();
            free(input);
            continue;
        }

        // To avoid hanging if grep is entered without the right arguments
        if (strcmp(tokens[0], "grep") == 0) {
            if (tokens[1] == NULL) {
                printf("grep: missing argument\n");                                                 
                continue;
                }
        }

        // General argument check (if the user inputs "cat" with no file, an error will arise)
        // I would have expected this to work without a specific implementation, but I got a hanging error without this
        if (CommandRequiresArguments(tokens[0]) && tokens[1] == NULL) {
            printf("%s: missing required arguments\n", tokens[0]);
            continue;                                           // Skip the rest of the loop if no arguments are provided
        }

        // Forking a child process to execute the user-given command
        pid_t pid = fork();
        if (pid == -1) {
            perror("Fork Failed . . . ");                                                           // Error case
            exit(1);
        }

        // Child process
        if(pid == 0) {
            // Handle redirection
            HandleRedirection(tokens);

            // Executing the command line using execvp
            if (execvp(tokens[0], tokens) == -1) {
                perror("Command Execution failed . . . ");                                          // Error case
                exit(1);                                        // Exiting the child process if the execvp fails
            }
        } else {                                                // Parent process
            // Wait for the child process to finish
            waitpid(pid, NULL, 0);
        }
    }

    // Free the no longer needed history queue
    for (int i =0; i < history_count; i++) {
        free(history[i]);
    }

    return 0;
}

// <----------------------------------------------------------------> PREVIOUSLY PROTOTYPED FUNCTIONS <------------------------------------------------------------->

// A function to trim unwanted characters from the input (newlines, spaces, etc.)
void TrimInput(char* input) {
    int length = strlen(input);
    while (length > 0 && (input[length - 1] == '\n' || input[length - 1] == '\r' || input[length - 1] == ' ')) {
        input[length - 1] = '\0';                               // Remove the trailing unwanted character
        length--;
    }
}

// A function to tokenize the input into the desired command and the arguments
void ParseUserInput(char* input, char** tokens) {
    char* token = strtok(input, " \t\n");                       // Tokenize the input by spaces, tabs, and newlines
    int i = 0;
    while (token != NULL) {
        tokens[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    tokens[i] = NULL;                                           // Null-terminate the token list
}

// A function to add the user's command to the history queue
void AddToHistory(const char *command) {
    // If the command is empty, don't process it (i.e. the user inputs " ")
    if (strlen(command) == 0 || strspn(command, " \t") == strlen(command)){
        return;
    }
    
    // Check to see if the command already exists in the history
    int found = -1;
    for (int i = 0; i < history_count; i++) {
        if (strcmp(history[i], command) == 0) {
            found = i;
            break;
        }
    }

    // If the command already exists in the history, remove it and reinsert it at the end
    if (found != -1) {
        free(history[found]);                                   // Free the memory at the found index
        for (int i = found; i < history_count - 1; i++) {
            history[i] = history[i + 1];                        // Shift elements to the left
        }
        history[history_count - 1] = NULL;                      // Avoids a dangling pointer (to avoid segmentation faults and corruption)
        history_count--;                                        // Reduce the history count
    }

    // If the history is full, remove the oldest command (shift everything to the left)
    if (history_count == HISTORY_SIZE) {
        free(history[0]);                                       // Free the memory of the oldest command
        for (int i = 0; i < HISTORY_SIZE - 1; i++) {
            history[i] = history[i + 1];                        // Shift elements to the left
        }
        history[history_count - 1] = NULL;
        history_count--;                                        // Reduce the history count
    }

    // Add the new command at the end of the queue
    history[history_count] = strdup(command);                   // Duplicate the new command
    history_count++;                                            // Increment the history count
}

// A function to display the command history
void PrintHistory() {
    for (int i = 0; i < history_count; i++) {
        printf("%d: %s\n", i + 1, history[i]);
    }
}

// A function to allow the user to execute a command from the history
void ExecuteCommandFromHistory(int index) {
    // Error - index is out of range
    if (index < 0 || index >= history_count) {
        printf("This command does not exist in the history . . . \n");
        return;
    }

    printf("Executing: %s\n", history[index]);
    char *tokens[MAX_TOKENS];
    char command[MAX_INPUT_SIZE];
    strcpy(command, history[index]);
    ParseUserInput(command, tokens);

    pid_t pid = fork();
    if (pid == -1) {
        perror("Fork failed . . . ");                                                               // Error case
        exit(1);
    }
    if (pid == 0) {
        if (execvp(tokens[0], tokens) == -1) {
            perror("Command execution failed . . . ");                                              // Error case
            exit(1);
        }
    } else {
        waitpid(pid, NULL, 0);
    }
}

// A function to handle input/output redirection
void HandleRedirection(char **tokens) {
    int i = 0;
    int input_redirect = -1, output_redirect = -1;
    char *input_file = NULL, *output_file = NULL;

    // Checking the tokens for input/output redirection
    while (tokens[i] != NULL) {
        if (strcmp(tokens[i], "<") == 0) {
            input_redirect = i;
            input_file = tokens[i + 1];
            tokens[i] = NULL;                                   // Remove the redirection operator from the tokens
        } else if (strcmp(tokens[i], ">") == 0) {
            output_redirect = i;
            output_file = tokens[i + 1];
            tokens[i] = NULL;                                   // Remove the redirection operator from the tokens
        }
        i++;
    }

    // Handle input redirection
    if (input_redirect != -1) {
        int fd_in = open(input_file, O_RDONLY);
        if (fd_in == -1) {
            perror("Input redirection failed . . . ");                                              // Error Case
            exit(1);
        }
        dup2(fd_in, STDIN_FILENO);                              // Redirect stdin to the file
        close(fd_in);
    }

    // Handle output redirection
    if (output_redirect != -1) {
        int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_out == -1) {
            perror("Output redirection failed . . . ");
            exit(1);
        }
        dup2(fd_out, STDOUT_FILENO);                            // Redirect stdout to the file
        close(fd_out);
    }
}

// Check if the command requires arguments
int CommandRequiresArguments(char* command) {
    // List of commands that require arguments -- more exist than this
    if (strcmp(command, "cat") == 0 || strcmp(command, "cp") == 0 || strcmp(command, "mv") == 0 || strcmp(command, "rm") == 0) {
        return 1;                                               // These commands require arguments
    }
    return 0;                                                   // Default: commands like 'ls', 'pwd', etc., do not require arguments
}

// A function responsible for suggesting the commands for tab autocompletion
char *CommandGenerator(const char *text, int state) {
    static int list_index, len;
    
    // A list of the commands I have allowed for tab autocompletion
    char *commands[] = {
        "cd", "ls", "pwd", "echo", "clear", "exit",
        "touch", "cat", "cp", "mv", "rm", "mkdir", "rmdir",
        "vim", "nano", "less", "more", "head", "tail",
        "ps", "top", "kill", "killall", "whoami",
        "ping", "uname", "hostname", "uptime", "df", "du", "free", "who",
        "grep","sort", "uniq",  "date", "history", NULL
    };

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    while (commands[list_index]) {
        if (strncmp(commands[list_index], text, len) == 0) {
            return strdup(commands[list_index++]);
        }
        list_index++;
    }
    return NULL;
}

char **CustomCompletion(const char *text, int start, int end) {
    // If not at the beginning, assume file/directory completion
    if (start != 0) {
        return rl_completion_matches(text, rl_filename_completion_function);
    } else {
        // Otherwise, complete as a command
        return rl_completion_matches(text, CommandGenerator);
    }
}

// Custom `ls` implementation - for user experience
void ExecuteLS() {
    struct dirent *entry;
    struct stat file_stat;

    // Open the current directory
    DIR *dir = opendir(".");
    if (!dir) {
        perror("ls");
        return;
    }

    // Read and print each entry in the directory
    while ((entry = readdir(dir)) != NULL) {
        stat(entry->d_name, &file_stat);

        // If it's a directory, print in blue
        if (S_ISDIR(file_stat.st_mode)) {
            printf("%s%s%s  ", COLOR_BLUE, entry->d_name, COLOR_RESET);
        } else {
            printf("%s  ", entry->d_name);
        }
    }
    printf("\n");
    closedir(dir);
}
