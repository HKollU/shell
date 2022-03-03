/*============Includes==========*/
#include <unistd.h> /*For read, write, fork, and execve operations*/
#include <string.h> /*For strlen, strtok, strcat, strcatn, strncomp, strcomp operations*/
#include <sys/wait.h>/*For waitpid, WIFEXITED, and WEXITSTATUS operations*/
#include <sys/stat.h> /* For struct stat, and statbuf struct*/
#include <stdlib.h> /* For environ, free, calloc, realloc, size_t, and exit operations*/
#include <sys/types.h> /*For pid_t datatype to store the PID*/
#include <ctype.h> /*For the isspace function*/

/*============Globals===========*/

const int STDIN = 0; /*Used to designate the location of STDIN*/
const int STDOUT = 1; /*Used to designate the location of STDOUT*/
const int STDERR = 2;
extern char** environ; /* The working environment the program operates in*/
const char * ptr; /*Used to designate the size of a character pointer for the creation of dynamic 2D arrays*/
char * PWD; /* The PWD the environment operates in, Used to search for binaries inside of the PWD */

/*============Prototypes==========*/

/*Desc:
    Function get_Input() takes a location to retrieve input, and a location to store input.
    It then dynamically allocates input from the input_location to create a dynamic string with that input.
    The string the end of the input is always catenated with a string that contains: "\t;\0".
    Input args : int input_location - the file stream the input is stored in (STDIN/OUT/ERROR/etc.)
                char * input - A string where the contents of the file stream will be stored.
    Returns : An string with the location of input.*/
char * get_Input(int input_location, char * input); 

/*----------------------------------------------------------------------------*/

/*Desc:
    Function normalize() takes a string and turns all of the 'spaces' into 'tabs' within the string.
    Input Args: char * input - The string which will be transformed
    Returns : The String which was transformed.*/    
char * normalize(char * input);

/*----------------------------------------------------------------------------*/

/*Desc:
    Function get_Paths() takes in a string known as "PATH" which contains the valid binary paths for programs
    to run. Once it has the path it chops up the PATH by the delimitor ':' to create an array of paths, each with
    a useable location of where a program may be found.
    Input Args: const char * PATH - The string which contains the valid binary paths for the current user.
    
    Returns : An array of strings. Each containing one of the PATHs contained in the variable "PATH".*/
char ** get_Paths(const char * PATH);

/*----------------------------------------------------------------------------*/

/*Desc:
    Function tokenize_Arguments() turns an argument string into an array of argument tokens as separated by a 'TAB' character.
    Input Args: char ** arguments - The location that the string tokens will be stored.
                char * the_args - The string that will be broken up into tokens.
    
    Returns: A list of potential paths that the user may have binaries stored within.*/
char ** tokenize_Arguments(char **arguments, char *the_args);

/*----------------------------------------------------------------------------*/

/*Desc:
    Function get_Exe_Path() Attempts to find whether or not a binary file exists within a list of
    valid paths that binaries may be found within the file system. If a file does exist, then its path is returned.
    Otherwise, NULL is returned.
    Input Args : char * exe_path - The location that the path to a particular command's executable is stored.
                char * command - The command whose binary will be searched for.
                char ** valid_paths - The list of paths that will be searched in for a particular binary.
    
    Returns: The path to a binary if it is found, NULL otherwise.*/
char * get_Exe_Path(char * exe_path, char * command, char ** valid_paths);

/*----------------------------------------------------------------------------*/

/*Desc:
    The function parse_arguments drives when a command is run or not run and how different commands issued to the shell are run.
    This contains the logic for the '||', '&&', and the ';' operators.  These operators must be separated from a string with a 
    tab in order to be identified.
    The '||' operator, skips the command on its right if the command on the left runs w/ no abnormalities.
            ie (untokenized input))
                echo 1\t||\techo 2
            would print: 1
            whereas
                lkjsdf\t||\t echo yup
            would print: yup 
    The '&&' operator only executes the command on its right if the command on the left hand side executes successfully.
        ie (untokenized input))
                echo 1 && echo 2
            would print: 1
                         2
            whereas
                oiasdj && echo 2
            would print: 2*/
void parse_arguments(char ** arg_tokens, char ** valid_paths);

/*----------------------------------------------------------------------------*/

/*Desc
    The function free_2D_Char() frees the memory of a 2D array of strings and the information contained therein.
    Input Args: char ** arr - the 2D array that will be freed. */

void free_2D_Char(char ** arr);

/*----------------------------------------------------------------------------*/

/*Desc
    The function parse_Input normalizes input, turns them into tokens, and then parses the input as arguments using the above functions.
    Input Args: char * input - A set string that will be parsed.
    */
void parse_Input(char * input);

/*----------------------------------------------------------------------------*/

/*Desc
    The split_Comm() function divides up semi-colon operations into command blocks. These blocks are then further divided up thereafter.
    Input Args: char ** commands - The location the command blocks will be stored
                char * the_args - the string that will be divided up.
    */
char ** split_Comm(char** commands, char * the_args); 


/*==========Functions==========*/

char * get_Input(int input_location, char * input)
{
    
    int i = 0;
    char * input_buffer = calloc(100, sizeof(char));
    int status = 0;
    memset(input_buffer, '\0', 100); 
    while((status = read(input_location,input_buffer,1))> 0)
    {
        if(input_buffer == NULL || input_buffer[0] == '\n' )
        {
            status=1;
            break;
        }
        input = realloc(input, (i+1) *sizeof(char) ); 
        memset(input+i, '\0', 1);
        input[i] = input_buffer[0];
        if(input[i] < 32 || input[i] > 126) 
            input[i] = '\t';
        i++;
    }
    if(status == 0)
    {   
        write(STDOUT, "\n",1);
        exit(EXIT_SUCCESS);
    }
    if(status < 0)
    {
        char * message = "<ERROR>: COULD NOT RETRIEVE INPUT FROM STDIN;\n";
        memset(input, '\0', strlen(input));
        write(STDERR,message, strlen(message));
        exit(EXIT_FAILURE);
        
    }
    else
    {
        input = realloc(input, (i+4) *sizeof(char) );
        input[i] = '\0';
        input = strcat(input, "\t;\0");
    }
    return input;
}

/*----------------------------------------------------------------------------*/

char * normalize(char * input)
{
    int i = 0;
    while(input != NULL && input[i] != '\0')
    {
        if(isspace(input[i]))
            input[i] = '\t';
        i++;
    }
    return input;
}

/*----------------------------------------------------------------------------*/

char ** get_Paths(const char * PATH)
{
    char ** valid_paths = NULL; 
    char * path_copy = calloc(strlen(PATH)+3, sizeof(char));
    char * curr_path; 
    int i = 0;
    valid_paths = calloc(strlen(PATH)+3, sizeof(ptr));
    strcpy(path_copy, PATH);
    curr_path = strtok(path_copy, ":"); 
    while (curr_path != NULL)
    {
        valid_paths = realloc(valid_paths, (i + 1) * sizeof(ptr));
        valid_paths[i] = calloc(strlen(curr_path) + 3, sizeof(char));
        strcpy(valid_paths[i], curr_path);
        valid_paths[i][strlen(curr_path)] = '/';
        valid_paths[i][strlen(curr_path) + 1] = '\0';
        i++;
        curr_path = strtok(NULL, ":");
    }
    valid_paths = realloc(valid_paths, (i + 1) * sizeof(ptr));
    valid_paths[i] = NULL;
    return valid_paths;
}

/*----------------------------------------------------------------------------*/

char ** split_Comm(char **arguments, char *the_args)
{
    char *token = strtok(the_args, ";"); 
    int i = 0; 
    arguments = calloc(strlen(the_args)+3, sizeof(ptr));
    while (token != NULL)
    {
        arguments = realloc(arguments, (i + 1) * sizeof(ptr));
        arguments[i] = calloc(strlen(token)+3, sizeof(char));
        memset(arguments[i], '\0',strlen(token)+3);
        strcpy(arguments[i], token);
        strcat(arguments[i], "\t;");
        token = strtok(NULL, ";");
        i++;

    }
    arguments = realloc(arguments, (i + 1) * sizeof(ptr));
    arguments[i] = NULL;
    return arguments;
}

/*----------------------------------------------------------------------------*/

char ** tokenize_Arguments(char **arguments, char *the_args)
{
    char *token = strtok(the_args, "\t"); 
    int i = 0; 
    arguments = calloc(strlen(the_args)+3, sizeof(ptr));
    while (token != NULL)
    {
        arguments = realloc(arguments, (i + 1) * sizeof(ptr));
        arguments[i] = calloc(strlen(token)+3, sizeof(char));
        memset(arguments[i], '\0',strlen(token)+3);
        strcpy(arguments[i], token);
        token = strtok(NULL, "\t");
        i++;

    }
    arguments = realloc(arguments, (i + 1) * sizeof(ptr));
    arguments[i] = NULL;
    return arguments;
}

/*----------------------------------------------------------------------------*/

char * get_Exe_Path(char * exe_path, char * command, char ** valid_paths)
{
    int i = 0;
    int exists = 0; 
    struct stat statbuf;
    char * failure_message = " :COMMAND NOT FOUND\n";
    PWD = getenv("PWD");
    exe_path = realloc(exe_path, (strlen(command) + strlen(PWD) + 3) * sizeof(char));
    exe_path = strcpy(exe_path, PWD);
    exe_path = strncat(exe_path, "/", strlen("/"));
    exe_path = strncat(exe_path, command, strlen(command));
    exists = stat(exe_path, &statbuf) + 1 == 1;
    if (exists)
        return exe_path;
    while (valid_paths[i] != NULL)
    {
        exe_path = realloc(exe_path, (strlen(command) + strlen(valid_paths[i]) + 3) * sizeof(char));
        exe_path = strcpy(exe_path, valid_paths[i]);
        exe_path = strncat(exe_path, command, strlen(command));
        exists = stat(exe_path, &statbuf) + 1 == 1;
        if (exists)
            return exe_path;
        i++;
    }
    write(STDOUT,command, strlen(command) );
    write(STDOUT, failure_message, strlen(failure_message));
    return NULL;
}

/*----------------------------------------------------------------------------*/

int run_Command(char * exe_path, char ** commands)
{
    pid_t pid;
    int status = 0;
    if((pid = fork())== 0)
    {
        execve(exe_path, commands, environ); 
        exit(EXIT_SUCCESS);
    }
    else if (pid > 0)
    {
        waitpid(pid,&status, 0);
        if(WIFEXITED(status))
            return WEXITSTATUS(status);
        else
        {
            write(STDERR, "ERROR OBTAINING STATUS CODE\n", strlen("ERROR OBTAINING STATUS CODE\n"));
            return -1;
        }
    }
    else
    {
        write(STDERR, "ERROR: COULD NOT FORK, TERMINATING..\n", strlen("ERROR: COULD NOT FORK, TERMINATING..\n"));
        exit(EXIT_FAILURE);
    }
    write(STDERR, "ERROR: JUMPED COMMAND\n", strlen("ERROR: JUMPED COMMAND\n"));
    return -1;

}

/*----------------------------------------------------------------------------*/

void parse_arguments(char ** arg_tokens, char ** valid_paths)
{
    char ** command_lines;
    char * exe_path = calloc(10, sizeof(char));
    int cmd_pos = 0;
    int skip_next = 0;
    int arg_pos = 0;
    int cmd_sts = 0;
    command_lines = calloc(1, sizeof(ptr));
    while(arg_tokens != NULL && arg_tokens[arg_pos] != NULL)
    {
        if(strncmp(arg_tokens[arg_pos], ";", 2) == 0)
        {

                if(command_lines != NULL && command_lines[0] != NULL)
                    exe_path = get_Exe_Path(exe_path, command_lines[0],valid_paths);/*end if*/
                else
                    cmd_sts = 19;
                if(exe_path != NULL && skip_next != 1)
                {
                    cmd_sts = run_Command(exe_path, command_lines);
                    memset(exe_path, '\0', strlen(exe_path));
                }
                else
                    cmd_sts = 1;
                if (skip_next == 1)
                    skip_next = 0;
                while(cmd_pos >= 0)
                {
                    command_lines[cmd_pos] = NULL;
                    cmd_pos--;
                }
                cmd_pos = 0;
                free(exe_path);
                exe_path = calloc(10, sizeof(char*));
        }
        else if (strncmp(arg_tokens[arg_pos], "&&", 3) == 0)
        {
            
            if(command_lines != NULL && command_lines[0] != NULL)
             {      
                    exe_path = get_Exe_Path(exe_path, command_lines[0],valid_paths);/*end if*/
                    cmd_sts = 2;
             }
                if(exe_path != NULL && cmd_sts != 0 && skip_next!= 1 )
                {
                    cmd_sts = run_Command(exe_path, command_lines);
                    memset(exe_path, '\0', strlen(exe_path));
                    
                }
                else if (skip_next == 1)
                {
                    skip_next = 0;
                    cmd_sts=0;
                }
                if(cmd_sts != 0)
                {
                    free(exe_path);
                    break;
                }
                cmd_pos = 0;
                free(exe_path);
                exe_path = calloc(10, sizeof(char*));
        }
        else if (strncmp(arg_tokens[arg_pos], "||", 3) == 0)
        {
            cmd_sts = 1;
            if(command_lines != NULL && command_lines[0] != NULL && skip_next!= 1)
                exe_path = get_Exe_Path(exe_path, command_lines[0],valid_paths);/*end if*/
            if(exe_path != NULL && skip_next != 1)
            {
                cmd_sts = run_Command(exe_path, command_lines);
                memset(exe_path, '\0', strlen(exe_path));
                
            }
            if(cmd_sts == 0)
            {
                skip_next = 1;
               
            }
            cmd_pos = 0;
            free(exe_path);
            exe_path = calloc(10, sizeof(char*));
        }
        else
        {
            command_lines = realloc(command_lines, (cmd_pos+2)*sizeof(ptr));
            command_lines[cmd_pos] = calloc(strlen(arg_tokens[arg_pos])+1, sizeof(char));
            command_lines[cmd_pos] = strcpy(command_lines[cmd_pos],arg_tokens[arg_pos]);
            command_lines[cmd_pos+1] = NULL;
            cmd_pos++;
        }
        arg_pos++;
    }

}

/*----------------------------------------------------------------------------*/

void parse_Input(char * input)
{
    char ** valid_paths = NULL;
    char ** arg_tokens = NULL;
    int i = 0;
    char ** comm = NULL;
    char * PATH = getenv("PATH");
    if(input == NULL || strlen(input) < 1)
        return;
    input = normalize(input);
    valid_paths = get_Paths(PATH);
    comm = split_Comm(comm, input);
    while(comm != NULL && comm[i] != NULL)
    {
        arg_tokens = NULL;
        arg_tokens = tokenize_Arguments( arg_tokens, comm[i]);
        parse_arguments(arg_tokens, valid_paths);
        free_2D_Char(arg_tokens);
        i++;

    }
    free_2D_Char(comm);
    free_2D_Char(valid_paths);
}

/*----------------------------------------------------------------------------*/

void free_2D_Char(char ** arr)
{
    int i = 0;
    if (arr == NULL)
        return;
    while(arr[i] != NULL)
    {
        memset(arr[i], '\0', strlen(arr[i]));
        free(arr[i]);
        i++;
    }
    if (arr != NULL)
        free(arr);
}       

/*=============Main============*/

int main(int argc, char ** argv)
{
    char * my_input = NULL;
    char * envstring = calloc(1, sizeof(char));
    char * end_cap = ">$ ";
    while(1)
    {
        envstring = realloc(envstring, (strlen(getenv("PWD"))+ 4 )*sizeof(char));
        envstring =strcpy(envstring, getenv("PWD"));        
        strcat(envstring, end_cap);
        write(STDOUT,envstring, strlen(envstring));
        my_input = calloc(100, sizeof(char));
        memset(my_input, '\0', 100);
        my_input = get_Input(STDIN, my_input);
        parse_Input(my_input);
        memset(my_input, '\0', strlen(my_input));
        free(my_input);
      
    }
    exit(EXIT_SUCCESS);

}
