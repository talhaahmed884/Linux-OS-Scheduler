#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sstream>
#include "PCB.h"
#include <sys/stat.h>
using namespace std;


int toInt(char*);       //to convert the character array to the integer
bool randBlock();       //for randomly blocking the process


int main(int argc, char* args [])
{
    int pipe_fd_ready[2], pipe_fd_run[2], pipe_fd_exit[2], pipe_fd_block[2];    //pipes between ready and running state, running and ready state, running and exit state, running and block state
    pipe_fd_ready[0] = toInt(args[0]);  //pipe for ready to run
    pipe_fd_ready[1] = toInt(args[1]);
    pipe_fd_run[0] = toInt(args[2]);        //pipe for run to ready
    pipe_fd_run[1] = toInt(args[3]);
    pipe_fd_exit[0] = toInt(args[4]);   //pipe for run to exit
    pipe_fd_exit[1] = toInt(args[5]);
    pipe_fd_block[0] = toInt(args[6]);  //pipe for run to block
    pipe_fd_block[1] = toInt(args[7]);

    PCB *proc = new PCB;        //for storing the information of the process
    int read_head = -1;      //stores the read value returned by the read
    bool blocked = false;       //becomes true if a process is blocked
    bool exit = false;          //becomes true if a process has exited
    int clock = 0;      //clock of the cpu

    while(1)
    {
        read_head = read(pipe_fd_ready[0],proc,sizeof(PCB));        //reads data if availabe in the pipe
        if(read_head != -1)
        {

            blocked = false;        //resetting the bool variables
            exit = false;
            int data = proc->burst;     //storing the burst of the process left
            data--;     //decrements the burst
            clock++;        //increment the clock
            sleep(1);       //depicts the tick of time

            if((clock%5 == 0) && (data!=0))         //if the blocked conditions are true and burst hasn't ended
            {
                if(randBlock())     //randomly decides if the process nedds to be blocked or not
                {
                    proc->burst = data;     //update the remaining burst and completed burst
                    proc->completedBurst += 1;
                    write(pipe_fd_block[1],proc,sizeof(PCB));       //dispatch to the blocked state
    
                    blocked = true;
                }
            }

            if(data > 0 && !blocked)        //if the process hasn't completed and not blocked
            {
                proc->burst = proc->burst - 1;      //update the completed burst and remaining burst of the process
                proc->completedBurst += 1;
                int temp=-1;
                write(pipe_fd_run[1],&temp,sizeof(int));    //sending signal to the ready state
                write(pipe_fd_run[1],proc,sizeof(PCB));         //dispatch the process to ready state
            }

            if(data == 0)           //if all the burst of process has been finished then it sends to the exit state
            {
                exit = true;
                proc->burst = proc->burst - 1;      //update the completed burst and remaining burst of the process
                proc->completedBurst += 1;
                write(pipe_fd_exit[1],proc,sizeof(PCB));        //dispatch to the exit state
            }

            if(!exit && blocked)        //sends signal to the ready state to send the next process for execution if process is blocked or exited
            {
                int temp = -1;
                write(pipe_fd_run[1],&temp,sizeof(int));
            }
            else if(exit && !blocked)
            {
                int temp = -1;
                write(pipe_fd_run[1],&temp,sizeof(int));
            }
        }
        else
        {
            clock++;        //increments clock normally when nothing is happening
            sleep(1);
        }
    }

    return 0;
}


int toInt(char* arr)        //to convert the character array to the integer
{
    stringstream toint;     //buffer for storing the data
    toint<<arr;             //store character array into the buffer
    int temp;       //for storing the inetger
    toint>>temp;            //for storing the data from buffer to the ineteger
    return temp;
}


bool randBlock()        //for randomly blocking the process
{
    srand(time(NULL));      //set the random time
    int block = rand()%2;           //calculates the random value for blocking
    return block;
}