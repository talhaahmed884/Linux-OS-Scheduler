#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sstream>
#include "PCB.h"
#include <sys/stat.h>
using namespace std;


void stimulator();      //function that starts the whole stimulator
char* toChar(int);      //to convert the integer in to a character array
bool strcmp(char*,string);      //compare the string with the character array
void readAlgo(char*, int*);        //read the algo from the pipe to start the execution for that algo only


int main()
{
    stimulator();           //function that starts the whole stimulator
    return 0;
}


void stimulator()       //function that starts the whole stimulator
{
    pid_t proc_New;     //stores id of the new process
    pid_t proc_Ready;       //stores the id of the ready process
    pid_t proc_Run;     //stores the id of the running process
    pid_t proc_Block;           //stores the id of the block process
    pid_t proc_Exit;            //stores the id of the exit process

    int pipe_fd_new[2];     //pipe for new and ready state
    pipe(pipe_fd_new);

    proc_New = fork();      //creating the new process
    if(proc_New == 0)           //new state calling implementation
    {
        close(pipe_fd_new[0]);      //closing the read head of the pipe for new and ready state
        char *pipe_fd_new_one = toChar(pipe_fd_new[0]);     //converting the file descriptors to character array to pass it through exec
        char *pipe_fd_new_two = toChar(pipe_fd_new[1]);
        execlp("./newProc",pipe_fd_new_one,pipe_fd_new_two,NULL);       //calling the new state
    }
    else
    {
        int pipe_fd_ready[2];       //pipe between ready and running state
        pipe(pipe_fd_ready);

        int pipe_fd_run[2];     //pipe between running and ready state
        pipe(pipe_fd_run);

        int pipe_fd_runBlock[2];        //pipe between ready and blocked state
        pipe(pipe_fd_runBlock);

        char algo[4];
        readAlgo(algo,pipe_fd_new);         //reading algo information

        proc_Ready = fork();        //creating ready process
        if(proc_Ready == 0)     //ready state calling implementation
        {
            close(pipe_fd_new[1]);      //closing write head of pipe between new and ready state
            close(pipe_fd_runBlock[1]);     //close write head of pipe between ready and blocked state
            close(pipe_fd_ready[0]);        //close read head of pipe between ready and running state
            close(pipe_fd_run[1]);          //close write head of pipe between running and ready state state

            char *pipe_fd_ready_one = toChar(pipe_fd_ready[0]);     //converting the file descriptors into character array to pass it through exec
            char *pipe_fd_ready_two = toChar(pipe_fd_ready[1]);
            fcntl(pipe_fd_run[0],F_SETFL,O_NONBLOCK);           //enabling non block io for pipe between running and ready state
            char *pipe_fd_run_one = toChar(pipe_fd_run[0]);
            char *pipe_fd_run_two = toChar(pipe_fd_run[1]);
            char *pipe_fd_new_one = toChar(pipe_fd_new[0]);
            char *pipe_fd_new_two = toChar(pipe_fd_new[1]);
            fcntl(pipe_fd_runBlock[0],F_SETFL,O_NONBLOCK);      //enabling non block io for pipe between blocked and ready state
            char *pipe_fd_runBlock_one = toChar(pipe_fd_runBlock[0]);
            char *pipe_fd_runBlock_two = toChar(pipe_fd_runBlock[1]);

            execlp("./readyProc",pipe_fd_ready_one,pipe_fd_ready_two,pipe_fd_run_one,pipe_fd_run_two,pipe_fd_new_one,pipe_fd_new_two,pipe_fd_runBlock_one,pipe_fd_runBlock_two,NULL);       //calling the ready state
        }
        else
        {
            int pipe_fd_exit[2];        //pipe between running and exit state
            pipe(pipe_fd_exit);

            int pipe_fd_block[2];       //pipe between running and blocked state
            pipe(pipe_fd_block);

            proc_Run = fork();      //creating running process
            if(proc_Run == 0)       //running process calling implementation
            {
                close(pipe_fd_ready[1]);        //closing file descriptors not required in the running state
                close(pipe_fd_exit[0]);
                close(pipe_fd_block[0]);
                close(pipe_fd_runBlock[0]);
                close(pipe_fd_runBlock[1]);
                close(pipe_fd_run[0]);

                char *pipe_fd_run_one = toChar(pipe_fd_run[0]);     //converting the file descriptors to character to be passed as arguments in exec
                char *pipe_fd_run_two = toChar(pipe_fd_run[1]);
                fcntl(pipe_fd_ready[0],F_SETFL,O_NONBLOCK);         //enabling nonblock io
                char *pipe_fd_ready_one = toChar(pipe_fd_ready[0]);
                char *pipe_fd_ready_two = toChar(pipe_fd_ready[1]);
                char *pipe_fd_exit_one = toChar(pipe_fd_exit[0]);
                char *pipe_fd_exit_two = toChar(pipe_fd_exit[1]);
                char *pipe_fd_block_one = toChar(pipe_fd_block[0]);
                char *pipe_fd_block_two = toChar(pipe_fd_block[1]);

                if(strcmp(algo,"FCFS"))     //checks for different algos and their exec calls respectively
                {
                    execlp("./FCFSrun",pipe_fd_ready_one,pipe_fd_ready_two,pipe_fd_run_one,pipe_fd_run_two,pipe_fd_exit_one,pipe_fd_exit_two,pipe_fd_block_one,pipe_fd_block_two,NULL);
                }
                else if(strcmp(algo,"RR"))
                {
                    execlp("./RRrun",pipe_fd_ready_one,pipe_fd_ready_two,pipe_fd_run_one,pipe_fd_run_two,pipe_fd_exit_one,pipe_fd_exit_two,pipe_fd_block_one,pipe_fd_block_two,NULL);
                }
                else if(strcmp(algo,"SJF"))
                {
                    execlp("./SJFrun",pipe_fd_ready_one,pipe_fd_ready_two,pipe_fd_run_one,pipe_fd_run_two,pipe_fd_exit_one,pipe_fd_exit_two,pipe_fd_block_one,pipe_fd_block_two,NULL);
                }
                else if(strcmp(algo,"SRTF"))
                {
                    execlp("./SRTFrun",pipe_fd_ready_one,pipe_fd_ready_two,pipe_fd_run_one,pipe_fd_run_two,pipe_fd_exit_one,pipe_fd_exit_two,pipe_fd_block_one,pipe_fd_block_two,NULL);
                }
                
            }
            else
            {
                proc_Exit = fork();     //creating the exit process
                if(proc_Exit == 0)      //exit processes calling implementation
                {
                    close(pipe_fd_exit[1]);     //closing file descriptors not required in the exit state
                    close(pipe_fd_runBlock[0]);
                    close(pipe_fd_runBlock[1]);

                    fcntl(pipe_fd_exit[0],F_SETFL,O_NONBLOCK);      //enabling the non block io
                    char *pipe_fd_exit_one = toChar(pipe_fd_exit[0]);       //converting the file descriptors to character to be passed as arguments in exec
                    char *pipe_fd_exit_two = toChar(pipe_fd_exit[1]);
                    execlp("./exitProc",pipe_fd_exit_one,pipe_fd_exit_two,NULL);
                }
                else
                {
                    close(pipe_fd_exit[0]); //closing pipe heads not required in the state
                    close(pipe_fd_exit[1]);

                    proc_Block = fork();        //creating process for blocked state
                    if(proc_Block == 0)         //blocked processes calling implementation
                    {
                        close(pipe_fd_block[1]);    //closing file descriptors not required in the state
                        close(pipe_fd_runBlock[0]);

                        fcntl(pipe_fd_block[0],F_SETFL,O_NONBLOCK);     //enabling non block io
                        char *pipe_fd_block_one = toChar(pipe_fd_block[0]);     //converting the file descriptors to character to be passed as arguments in exec
                        char *pipe_fd_block_two = toChar(pipe_fd_block[1]);
                        char *pipe_fd_runBlock_one = toChar(pipe_fd_runBlock[0]);
                        char *pipe_fd_runBlock_two = toChar(pipe_fd_runBlock[1]);
                        execlp("./blockedProc",pipe_fd_block_one,pipe_fd_block_two,pipe_fd_runBlock_one,pipe_fd_runBlock_two,NULL);
                    }
                    else
                    {
                        close(pipe_fd_block[0]);    //closing ends of pipes not used
                        close(pipe_fd_block[1]);
                        close(pipe_fd_runBlock[0]);
                        close(pipe_fd_runBlock[1]);
                    }
                }
            }
        }
    }

    return;
}


char* toChar(int var)          //funtion that converts integer to Character array
{
    stringstream tochar;    //stream buffer for storin the data
    tochar<<var;            //storing integer in a buffer
    char *temp = new char[tochar.str().size()];         //initializing the character array
    for(int a=0;a<tochar.str().size();a++)
    {
        temp[a]=tochar.str()[a];            //storing the data in the character array
    }
    return temp;
}


void readAlgo(char *algo, int *pipe_fd)     //read the algo from the pipe to start the execution for that algo only
{
    close(pipe_fd[1]);
    read(pipe_fd[0],algo,4);        //reads data from the new state pipe for calling algo state

    return;
}


bool strcmp(char* str1, string str2)              //compared the character array and string
{
    int a=0,b=0;
    for(; str1[a]!='\0' || str2[b]!='\0'; a++,b++)          //compares the string with the character array untill one of the iterators have reached the size of any entity
    {
        if(str1[a]!=str2[b])
        {
            return false;               //returns false at the first mismatch
        }
    }
    if(a!=b)            //returns false if the size doesn't match
    {
        return false;
    }
    else
    {
        return true;
    }
}