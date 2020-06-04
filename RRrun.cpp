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
    int pipe_fd_ready[2], pipe_fd_run[2], pipe_fd_exit[2], pipe_fd_block[2];        //pipes between ready and running state, running and ready state, running and exit state, running and block state
    pipe_fd_ready[0] = toInt(args[0]);      //Pipe for ready to run state
    pipe_fd_ready[1] = toInt(args[1]);
    pipe_fd_run[0] = toInt(args[2]);        //pipe for run to ready
    pipe_fd_run[1] = toInt(args[3]);
    pipe_fd_exit[0] = toInt(args[4]);       //pipe for run to exit
    pipe_fd_exit[1] = toInt(args[5]);
    pipe_fd_block[0] = toInt(args[6]);      //pipe for run to block
    pipe_fd_block[1] = toInt(args[7]);

    PCB *proc = new PCB;        //for storing the information of the process
    int read_head = -1;     //stores the read value returned by the read
    int cpu_burst = 0;      //stores the burst of the process done by cpu
    int quantum = 0;        //stores the quantum for algo
    bool blocked = false;       //becomes true when the process is blocked
    bool exit = false;      //becomes true when a process has exited
    
    while(read_head == -1)      //reads data for quantum
    {
        read_head = read(pipe_fd_ready[0],&quantum,sizeof(int));
    }

    while(1)
    {
        read_head = read(pipe_fd_ready[0],proc,sizeof(PCB));        //check if data is available and read it from pipe
        if(read_head != -1)
        {
            cpu_burst = 0;      //resets burst for each process

            blocked = false;        //reset blocked value
            exit = false;           //reset exit value
            int data = proc->burst;     //stores the burst of process left
            while(data>0)           //run untill burst is zero
            {
                if(cpu_burst == quantum)        //if quantum is reached breaks
                {
                    break;
                }

                data--;     //decrements burst
                cpu_burst++;        //increments the time slice
                sleep(1);

                if((cpu_burst%5 == 0) && (data!=0))     //check if the blocked condition is true
                {
                    if(randBlock())     //randomly checks if process would be blocked
                    {
 
                        proc->burst = data;     //update the burst left and completed burst
                        proc->completedBurst += cpu_burst;
                        write(pipe_fd_block[1],proc,sizeof(PCB));       //dispatch to the blocked state

                        blocked = true;
                        break;
                    }
                }
            }

            if(data > 0 && !blocked)        //if process isn't blocked and burst isn't completed
            {
                proc->burst = proc->burst - cpu_burst;      //update the burst left and completed burst
                proc->completedBurst += cpu_burst;
                int temp=-1;
                write(pipe_fd_run[1],&temp,sizeof(int));        //sending signal to ready state
                write(pipe_fd_run[1],proc,sizeof(PCB));     //dispatch process to ready state
            }

            if(data == 0)       //if burst is completed
            {
                exit = true;
                proc->burst = proc->burst - cpu_burst;      //update the burst left and completed burst
                proc->completedBurst += cpu_burst;
                write(pipe_fd_exit[1],proc,sizeof(PCB));        //dispatch to the exit state
            }

            if(!exit && blocked)        //if process is blocked or dispatch to the exit state, send signal to ready state to dispatch new process to running state
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
    }

    return 0;
}


int toInt(char* arr)        //to convert the character array to the integer
{
    stringstream toint;     //buffer for storing data
    toint<<arr;     //moving data to buffe
    int temp;
    toint>>temp;        //moving data from buffer to integer
    return temp;
}


bool randBlock()        //for randomly blocking the process
{
    srand(time(NULL));      //set random time
    int block = rand()%2;       //randomly selects value for blocking process
    return block;
}