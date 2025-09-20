#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAX_INPUT 100

// stores command to be executed
typedef struct
{
    char *argv[MAXARG];
    char *endBuffer;
    int endOfLine;
}
execCommand;


char WHITESPACE[] = " \t\r\n\v";
char SYMBOLS[] = "<|>;";

// function prototypes
void fileDescriptorsCheck();
int getCommandFromUser(char *buffer, int sizeOfBuffer);
char *findBeginningOfString(char *buffer);
void execCD(char *buffer);
int execFork(void);
void parseCommand(char *buffer);
void runCommand(execCommand *buffer);
void panic(char *errMessage);
execCommand *mallocExecCommand();
void tokenize(char *buffer, execCommand *command);
void parseLine(execCommand *command);
void walkCommand(execCommand *command, int beginningOfCommand, int endOfCommand);
void parseExec(execCommand *command, int beginningOfCommand, int endOfCommand);
execCommand *makeSubCommand(execCommand *mainCommand, int beginningOfSubCommand, int endOfSubCommand);
void runPipelines(execCommand *commands[], int numPipes);


int main(void)
{
    char inputBuffer[MAX_INPUT];       // set up a buffer for the inputted commands

    // check file descriptors
    fileDescriptorsCheck();

    char *beginningPosition;

    // repeating input and execution cycle from user until terminated
    while (getCommandFromUser(inputBuffer, sizeof(inputBuffer)) >= 0)       
    {
        // find the beginning of the command (could be whitespace chars at the beginning)
        beginningPosition = findBeginningOfString(inputBuffer);

        if (beginningPosition[0] == 'c' && beginningPosition[1] == 'd' && beginningPosition[2] == ' ')       // check for "cd" input to shell
        {
            execCD(beginningPosition);        // execute cd command in parent ONLY
            continue;                   // begin next input and execute sequence
        }
        else if (beginningPosition[0] == '\n' || beginningPosition[0] == '\0')          // check if nothing was inputted
        {
            continue;
        }

        if (execFork() == 0)            // split code into 2 processes and execute the child process         
        {
            parseCommand(beginningPosition);    // execute command(s) given
        }
        wait(0);           // wait for the child process to end
    }

    exit(0);        // terminate
}


/*      FILE DESCRIPTORS CHECK     */

// checks file descriptors
// returns 1 if file descriptors are bad
void fileDescriptorsCheck()
{
    int fileDescriptors;

    // check to see if at least 3 file descriptors are open
    while ((fileDescriptors = open("console", O_RDWR)) >=0)      
    {
        if (fileDescriptors >= 3)               
        {
            close(fileDescriptors);
            break;
        }
    }
}

 
/*      GET COMMAND FROM USER      */

// outputs a promt message ">>>" to the user
// will expect a command input from the user, or wait
int getCommandFromUser(char *buffer, int sizeOfBuffer)
{
    // output the prompt to std
    char *outputPrompt = ">>>";
    write(2, outputPrompt, (sizeof(outputPrompt)));     // outputting to stderr

    // initialise/reset buffer to store new input
    memset(buffer, 0, sizeOfBuffer);

    // get input from user and store to the input buffer
    gets(buffer, sizeOfBuffer);

    // check for no input
    if (buffer[0] == 0)
    {
        return -1;
    }
    return 0;
}


/*      FIND BEGINNING OF COMMAND      */

// function returns a representative integer that leads to the beginning of the command
char *findBeginningOfString(char *buffer)
{
    // loop through buffer until a char that isnt a whitespace is found 
    char *beginningOfString;
    beginningOfString = buffer;
    char *ptr;
    while ((ptr = strchr(WHITESPACE, *buffer)))
    {
        // check if nothing was inputted
        if (*buffer == '\0')
        {
            break;
        }

        // check if the position exceeds the limits of the buffer
        if ((buffer - beginningOfString) == strlen(beginningOfString))
        {
            break;
        }
        buffer++;
    }
    return buffer;
}


/*      EXECUTE CHANGE DIRECTORY      */

// runs the cd command
void execCD(char *buffer)
{
    // remove the '\n' at the end of the buffer
    buffer[strlen(buffer) - 1] = 0;
    
    // obtain file path from command (starting from ' ' after "cd" input)
    char *positionOfString;
    positionOfString = buffer + 2;
    positionOfString = findBeginningOfString(positionOfString);


    // execute the change directory command with given filename
    // and check if the execution was successful
    if (chdir(positionOfString) < 0)
    {
        fprintf(2, "cannot cd %s\n", positionOfString);
    }
}


/*      EXECUTE AND CHECK FOR VALID FORK      */

// function checks and executes a fork command
// runs checks to see if the fork is successfully made
// returns process ID, or runs panic if failed
int execFork(void)
{
    // run fork
    int processID;
    processID = fork();

    // check for fork success and return the process ID
    if (processID == -1)
    {
        panic("fork");
    }
    return processID;
}


/*      RUN COMMAND     */

// executes a command
void runCommand(execCommand *command)
{
    exec(command->argv[0], command->argv);
    fprintf(2, "exec %s failed\n", command->argv[0]);
    exit(0);
}


/*      PANIC     */

// outputs to stderr
// closes program
void panic(char *errMessage)
{
    fprintf(2, "%s\n", errMessage);
    exit(1);
}


/*      SET UP EXEC COMMAND     */

// sets up and returns the execCommand struct
execCommand *mallocExecCommand()
{
    // allocate memory and check for success
    execCommand *command = (execCommand *) malloc(sizeof(execCommand));
    if (command == 0)
    {
        panic("malloc");
    }

    // allocate memory for array
    for (int argc = 0; argc < MAXARG; argc++)
    {
        // allocate memory for the string to store to and check for success
        command->argv[argc] = (char *) malloc(MAX_INPUT);
        if (command->argv[argc] == 0)
        {
            panic("malloc");
        }

        // set all values in the string to NULL
        memset(command->argv[argc], 0, MAX_INPUT);
    }
    return command;
}


/*      PARSE COMMAND     */

// executes the commands inside the input buffer
void parseCommand(char *buffer)
{
    // set up pointers
    execCommand *command = mallocExecCommand();

    // set up pointer to the end of the string (null char)
    command->endBuffer = buffer + strlen(buffer);
    
    // run tokeniser on entire buffer
    tokenize(buffer, command);

    // begin creating tree
    parseLine(command);
}


/*      TOKENISER FUNCTION      */

// tokenises the buffer
void tokenize(char *buffer, execCommand *command) 
{
    int argc = 0;
    int charCounter;

    // looping until a NULL char is found
    while (*buffer != '\0') 
    {
        charCounter = 0;

        // skip leading whitespace
        while (strchr(WHITESPACE, *buffer))
        {
            if (buffer > command->endBuffer)
            {
                break;
            }
            buffer++;
        }

        // if any char is found
        if (*buffer != '\0') 
        {
            // save the char to the command->argv array if it isnt a whitespace or NULL
            while (strchr(WHITESPACE, *buffer) == 0  && *buffer != '\0') 
            {
                // if a token was found in the buffer, save that to argv
                if (strchr(SYMBOLS, *buffer))
                {  
                    // checking if there was no spaces between input char and token, then move to next argv 
                    if (charCounter)
                    {
                        argc++;
                    }
                    charCounter = 0;
                    command->argv[argc][charCounter] = *buffer;
                    buffer++;

                    // dont want to read in any chars after the token, to the same argv
                    break;
                }

                command->argv[argc][charCounter] = *buffer;
                buffer++;
                charCounter++;
            }

            argc++;
        }
    }
    // set the last arguement to a null terminating char
    command->argv[argc] = '\0';
}


/*      PARSE LINE      */

// parses a line
// runs commands related to each line
void parseLine(execCommand *command)
{
    // store indexes for where a semi-colon is
    int listForSemiColon[MAXARG];
    int semiColonCounter = 0;

    // find and store indexes for every semi-colon found from command
    for (int argc = 0; argc < MAXARG; argc++)
    {
        if (command->argv[argc][0] == ';')
        {
            listForSemiColon[semiColonCounter] = argc;
            semiColonCounter++;
        }
        // accounting for the eventuality of reaching the end of the command buffer
        else if (command->argv[argc][0] == '\0')
        {
            listForSemiColon[semiColonCounter] = argc - 1;
            semiColonCounter++;
            break;
        }
    }

    // create a list of subcommands to execute
    execCommand *commandArray[MAXARG];

    int beginningOfSubCommand = 0;
    int endOfSubCommand;

    execCommand *subCommand;

    // grab the index values of the subcommand, make a new subcommand, and save it to the array
    for (int commandNo = 0; commandNo < semiColonCounter; commandNo++)
    {
        endOfSubCommand = listForSemiColon[commandNo] - 1;
        subCommand = makeSubCommand(command, beginningOfSubCommand, endOfSubCommand);
        commandArray[commandNo] = subCommand;

        // update position for next subcommand
        beginningOfSubCommand = endOfSubCommand + 2;
    }
    
    // begin executing commands separated by the semi-colon
    for (int commandNo = 0; commandNo < semiColonCounter; commandNo++)
    {
        if (execFork() == 0)
        {
            walkCommand(commandArray[commandNo], 0, commandArray[commandNo]->endOfLine);
        }
        wait(0);
    }
}


/*      WALKING COMMAND    */

// execute exec, pipes, redirect according to whatever is in the command
void walkCommand(execCommand *command, int beginningOfCommand, int endOfCommand)
{
    // stores indexes to where a pipe is
    int pipeCommands[MAXARG];
    int pipeCounter = 0;

    // Check if the command contains a pipe (|)
    for (int argc = 0; argc < MAXARG; argc++)
    {
        if (command->argv[argc][0] == '|')
        {
            pipeCommands[pipeCounter] = argc;
            pipeCounter++;
        }

        // accounting for end of command
        if (command->argv[argc][0] == '\0')
        {
            pipeCommands[pipeCounter] = endOfCommand + 1;
            pipeCounter++;
            break;
        }
    }
    
    // create a list to store all pipe subcommands
    execCommand *commandArray[MAXARG];

    int beginningOfSubCommand = beginningOfCommand;
    int endOfSubCommand;
    execCommand *subCommand;

    // grab the index values of the subcommand, make a new subcommand, and save it to the array
    for (int commandNo = 0; commandNo < pipeCounter; commandNo++)
    {
        endOfSubCommand = pipeCommands[commandNo] - 1;
        subCommand = makeSubCommand(command, beginningOfSubCommand, endOfSubCommand);
        commandArray[commandNo] = subCommand;
        
        // update position for next subcommand
        beginningOfSubCommand = endOfSubCommand + 2;
    }

    // check if there are pipes to execute, and walk the correct path if there is
    if (pipeCounter - 1 != 0) 
    {
        runPipelines(commandArray, pipeCounter - 1);
    } 
    else 
    {
        parseExec(command, beginningOfCommand, endOfCommand);
    }
}


/*      EXECUTE PIPE COMMAND      */

// executes pipe commands using inverse recursion
void runPipelines(execCommand *commands[], int numPipes) 
{
    // set up file descriptors for the pipe
    // and check for success
    int fds[2];
    if (pipe(fds) < 0) 
    {
        panic("Pipe creation error\n");
    }

    // run a fork and execute as a child process
    if (execFork() == 0) 
    {
        // redirect output of the command to the pipe 
        close(1);
        dup(fds[1]);
        close(fds[0]);
        close(fds[1]);

        // depending on what the value of numPipes is, either
        // recursively run the command
        if (numPipes > 1)
        {
            runPipelines(commands, numPipes - 1);
        }
        // execute the last (actually first) command
        else if (numPipes == 1)
        {
            parseExec(commands[numPipes - 1], 0, commands[numPipes - 1]->endOfLine);
        }
        // panic on error
        else
        {
            panic("pipe index");
        }
    }
    // and for the parent command
    else
    {
        // redirect the input of the command to the pipe
        close(0);
        dup(fds[0]);
        close(fds[0]);
        close(fds[1]);

        // and execute that command
        parseExec(commands[numPipes], 0, commands[numPipes]->endOfLine);           
    } 

    // close file descriptors once complete
    close(fds[0]);
    close(fds[1]);

    // wait for the child to finish
    wait(0);   
}


/*      PARSE EXECUTION COMMAND     */

// runs execute on commands
// handles I/O redirects
void parseExec(execCommand *command, int beginningOfCommand, int endOfCommand)
{
    // store indexes for where an arrow is
    int indexEndOfCommand = endOfCommand;
    int arrowCounter = 0;

    // find and store indexes for every redirect found from command
    for (int argc = 0; argc < MAXARG; argc++)
    {
        // if an arrow was found in the command
        if (strchr("<>", command->argv[argc][0]))
        {
            // set the end of tehe command to be 1 less than where the arrow was found
            if (indexEndOfCommand == endOfCommand)
            {
                indexEndOfCommand = argc - 1;
            }

            arrowCounter++;

            // if no argument affter arrow is given, panic
            if (strchr(SYMBOLS, command->argv[argc + 1][0]) || command->argv[argc + 1] == 0)
            {
                panic("missing file for redirection");
            }

            // for the found arrow, redirect either to
            switch (command->argv[argc][0])
            {
                // input from file
                case '<':
                    close(0);
                    if (open(command->argv[argc + 1], O_RDONLY) < 0)
                    {
                        fprintf(2, "open %s failed\n", command->argv[argc + 1]);
                        exit(1);
                    }
                    break;

                // output to file
                case '>':
                    close(1);
                    if (open(command->argv[argc + 1], O_WRONLY|O_CREATE|O_TRUNC) < 0)
                    {
                        fprintf(2, "open %s failed\n", command->argv[argc + 1]);
                        exit(1);
                    }
                    break;
                
                default:
                    break;
            }
        }
    }

    // find the command to be run (minus the arrow)
    execCommand *subCommand = makeSubCommand(command, beginningOfCommand, indexEndOfCommand);
    // run command
    runCommand(subCommand);
}

/*      MAKE NEW SUBCOMMAND     */

// makes a subcommand given the main command and indexes to look between
// returns the subcommand
execCommand *makeSubCommand(execCommand *mainCommand, int beginningOfSubCommand, int endOfSubCommand)
{
    // make a new dynaamically allocated subcommand
    execCommand *subCommand = mallocExecCommand();

    int argcTracker = 0;

    // load subcommand program and args
    for (int subCommandCounter = beginningOfSubCommand; subCommandCounter <= endOfSubCommand; subCommandCounter++) 
    {
        strcpy(subCommand->argv[argcTracker], mainCommand->argv[subCommandCounter]);
        argcTracker++;
    }

    // set the end of the subcommand to NULL terminating char
    subCommand->endOfLine = argcTracker - 1;
    subCommand->argv[argcTracker] = '\0';

    return subCommand;
}