#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

void execute_line(char *line, char *args[], int c_count)
{
    // create and check for a fork
    int f = fork();
    if (f < 0)
    {
        fprintf(2, "xargs: fork failed");       // 2 = output to stderr
    }
    else if (f == 0)
    {
        // execute child
        // execute a line
        // call exec

        int ws = 1;         // variable for checking start of word of bool type
        char **a, *l;
        a = &args[c_count];
        l = line;

        while (*line != '\0')
        {
            if (*l != ' ')
            {
                if (ws)
                {
                    *a = l;
                    ws = 0;      // set word bool to 0
                }
            }
            else
            {
                if (!ws)
                {
                    *l = '\0';       // indicate the end of the line
                    a++;
                    ws = 1;
                }
            }
        }
        exec(args[0], args);
        l++;
    }
    else
    {
        // parent process
        wait(0);             // waiting for child to complete
    }
}


int main(int argc, char *argv[])
{
    // 1. Read in the command line from argv
    // 2. for each line of standard input
    // 3. execute the command supplied

    char *args[MAXARG] = { 0 };
    char buf[512];              // this is an optional size. you can make it dynamic
    char *b = buf;

    // for all characters after the 1st word inputted on the command line
    for (int i = 1; i < argc; i++)
    {
        args[i - 1] = argv[i];
    }
    
    // find the command by cycling through the buffer, with a ponter called b
    while (read(0, b, 1) > 0)
    {
        // if the command has reached a "new word" in the input
        if (*b == '\n')
        {
            *b = '\0';
            // execute the line
            execute_line(buf, args, argc-1);
            b = buf;
        }
        else
        {
            b++;
        }
    }
    // terminate
    exit(0);
}