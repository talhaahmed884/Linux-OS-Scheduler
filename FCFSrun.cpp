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
    pipe_fd_ready[0] = toInt(args[0]);  //read head of pipe between ready and running state
    pipe_fd_ready[1] = toInt(args[1]);  //write head of pipe between ready and running state
    pipe_fd_run[0] = toInt(args[2]);    //read head of pipe between running and ready state
    pipe_fd_run[1] = toInt(args[3]);    //write head of pipe between running and ready state
    pipe_fd_exit[0] = toInt(args[4]);   //read head of pipe between running and exit state
    pipe_fd_exit[1] = toInt(args[5]);   //write head of pipe between running and exit state
    pipe_fd_block[0] = toInt(args[6]);  //read head of pipe between running and block state
    pipe_fd_block[1] = toInt(args[7]);  //write head of pipe between running and block state

    PCB *proc = new PCB;        //for storing the information of the process
    int read_head = 0;      //stores the read value returned by the read
    int cpu_burst = 0;      //stores the burst of the process done by cpu

    while(1)
    {
        read_head = read(pipe_fd_ready[0],proc,sizeof(PCB));        //reads data if availabe in the pipe
        if(read_head != -1)
        {
            cpu_burst = 0;      //resetting the storage area for new process

            int data = proc->burst;     //storing the burst of the process left
            while(data>0)
            {
                data--;    //decrements the burst
                cpu_burst++;        //increment the burst
                sleep(1);       //depicts the tick of time

                if((cpu_burst%5 == 0) && (data!=0))     //if the blocked conditions are true and burst hasn't ended
                {
                    if(randBlock())     //randomly decides if the process nedds to be blocked or not
                    {
                        proc->burst = data;     //store the remaining burst lefts
                        proc->completedBurst += cpu_burst;      //stores the completed burst
                        write(pipe_fd_block[1],proc,sizeof(PCB));       //write the process information into the pipe

                        break;      //breaks the inside loop
                    }
                }
            }

            if(data == 0)       //if all the burst of process has been finished then it sends to the exit state
            {
                proc->burst = proc->burst - cpu_burst;      //update the values of burst of the process
                proc->completedBurst += cpu_burst;      //update the completed burst value
                write(pipe_fd_exit[1],proc,sizeof(PCB));        //writes it in the pipe
            }

            int temp = -1;      //signal for the ready state
            write(pipe_fd_run[1],&temp,sizeof(int));        //sends signal to the ready state to send the next process for execution
        }
    }
    return 0;
}


int toInt(char* arr)        //to convert the character array to the integer
{
    stringstream toint;     //buffer for storing the data
    toint<<arr;     //store character array into the buffer
    int temp;       //for storing the inetger
    toint>>temp;        //for storing the data from buffer to the ineteger
    return temp;
}


bool randBlock()        //for randomly blocking the process
{
    srand(time(NULL));      //set the random time
    int block = rand()%2;       //calculates the random value for blocking
    return block;
}