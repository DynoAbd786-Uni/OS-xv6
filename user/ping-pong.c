#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int main (int argc, char *argv[])
{
    int pipefd[2]; // File descriptors for the pipe
    char byte;

    if (pipe(pipefd) == -1) 
    {
        fprintf(2, "Failed to create a pipe.\n");
        exit(1);
    }

    int pid = fork(); // Create a child process

    if (pid < 0) 
    {
        fprintf(2, "Fork failed.\n");
        exit(1);
    }

    // when the fork function is executed, the code kinda splits into 2 processes
    // 1 with the child process and one with the parent
    // the if statement that follows is dependant on the 2 pid's that each process has

    if (pid == 0) { // Child process
        close(pipefd[1]); // Close the write end
        read(pipefd[0], &byte, sizeof(byte));
        printf("%d: received ping\n", getpid());
        close(pipefd[0]);
        exit(0);
    } else { // Parent process
        close(pipefd[0]); // Close the read end
        printf("%d: received pong\n", getpid());
        write(pipefd[1], &byte, sizeof(byte));
        close(pipefd[1]);
        int status;
        wait(&status);
        exit(0);
    }
}