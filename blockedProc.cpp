#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sstream>
#include "PCB.h"
#include <sys/stat.h>
#include <queue>
#include <vector>
#include <pthread.h>
using namespace std;


int toInt(char*);       //to convert the character array to the integer
void updateBlocked();        //update the blocked time and waiting time of the blocked processes
void getunBlocked(vector<PCB>*);        //dispatch the unblocked processes to the ready state
void *inputQueue(void*);        //for updating the queue with processes that are blocked with input, randBlock = 0
void *outputQueue(void*);       //for updating the queue with processes that are blocked with output, randBlock = 1
void *printQueue(void*);        //for updating the queue with processes that are blocked with print, randBlock = 2
int randQueue();        //randomly generates number to point the queue


queue<PCB>* inputqueue = new queue<PCB>;
queue<PCB>* outputqueue = new queue<PCB>;
queue<PCB>* printqueue = new queue<PCB>;



int main(int argc, char* args [])
{
    int pipe_fd_block[2], pipe_fd_runBlock[2];
    pipe_fd_block[0] = toInt(args[0]);      //read head of the pipe between run and block state
    pipe_fd_block[1] = toInt(args[1]);      //write head of the pipe between run and block state
    pipe_fd_runBlock[0] = toInt(args[2]);   //read head of the pipe between ready and block state
    pipe_fd_runBlock[1] = toInt(args[3]);   //write head of the pipe between ready and block state

    vector<PCB>* unblocked_procs = new vector<PCB>;     //stores the unblocked processes if processes are more than one
    int read_head = 0;          //stores the reading value returned by read command
    PCB *proc = new PCB;        //for storing the information of receiving or sending process
    int *size = new int;        //stores the number of unblocked processes
    int randBlock = -1; //stores the random value for the queue where the process would be stored, 0=inputQueue, 1=outputQueue, 2=printQueue

    pthread_t thread_one, thread_two, thread_three;     //three threads for maintaining three queues indepndently


    while(1)
    {
        read_head = read(pipe_fd_block[0],proc,sizeof(PCB));        //checks if pipe has any data
        if(read_head != -1)
        {
            proc->blocked = (rand()%(25-15+1))+15;      //calculates the random value for blocked process
            
            randBlock = randQueue();        //randomly choosing the blocked queue

            if(randBlock == 0)
            {
                pthread_create(&thread_one,NULL,inputQueue,(void*)(proc));       //for input queue
                pthread_join(thread_one,NULL);
            }
            else if(randBlock == 1)
            {
                pthread_create(&thread_two,NULL,outputQueue,(void*)(proc));      //for output queue
                pthread_join(thread_two,NULL);
            }
            else if(randBlock == 2)
            {
                pthread_create(&thread_three,NULL,printQueue,(void*)(proc));     //for print queue
                pthread_join(thread_two,NULL);
            }
            
        }

        updateBlocked();       //update the blocked time and waiting time of the blocked processes

        getunBlocked(unblocked_procs);    //dispatch the unblocked processes to the ready state

        *size = unblocked_procs->size();        //gets the size og unblocked processes

        if(*size > 0)           //becomes true only when there is any process unblocked
        {
            write(pipe_fd_runBlock[1],size,sizeof(int));    //writing the number of unblocked processes on pipe

            for(int a=0;a<*size;a++)
            {
                *proc = unblocked_procs->back();  unblocked_procs->pop_back();      //getting the unblocked processes
                write(pipe_fd_runBlock[1],proc,sizeof(PCB));            //writing the process on pipe
            }
        }
    }

    return 0;
}


int toInt(char* arr)       //converts character array into the integer
{
    stringstream toint;
    toint<<arr;     //storing the character into the buffer
    int temp;
    toint>>temp;            //moving data from the buffer into an integer
    return temp;
}


void updateBlocked()       //update the remaining blocked time and waiting time of the blocked processes
{
    sleep(1);

    if(!inputqueue->empty())        //updating the waiting time of the front process in the inputqueue
    {
        inputqueue->front().blocked -= 1;       //updating the blocked time
        if(inputqueue->front().blocked < 0)     //in case blocked value becomes smaller than 0
        {
            inputqueue->front().blocked = 0;        //reset it to zero
        }
    }

    if(!outputqueue->empty())           //updating the waiting time of the front process in the outputqueue
    {
        outputqueue->front().blocked -= 1;      //updating the blocked time
        if(outputqueue->front().blocked < 0)        //in case blocked value becomes smaller than 0
        {
            outputqueue->front().blocked = 0;       //reset it to zero
        }
    }

    if(!printqueue->empty())            //updating the waiting time of the front process in the printqueue
    {
        printqueue->front().blocked -= 1;   //updating the blocked time
        if(printqueue->front().blocked < 0)     //in case blocked value becomes smaller than 0
        {
            printqueue->front().blocked = 0;        //reset it to zero
        }
    }

    return;
}


void getunBlocked(vector<PCB> *unblocked)
{
    PCB proc;
    if(!inputqueue->empty())        //checking whether the queue is empty
    {
        if(inputqueue->front().blocked == 0)       //if the process at the front has completed its blocked time
        {
            proc = inputqueue->front(); inputqueue->pop();      //pop the process and push it on the unblocked process vector
            unblocked->push_back(proc);
        }
    }

    if(!outputqueue->empty())        //checking whether the queue is empty
    {
        if(outputqueue->front().blocked == 0)       //if the process at the front has completed its blocked time
        {
            proc = outputqueue->front(); outputqueue->pop();      //pop the process and push it on the unblocked process vector
            unblocked->push_back(proc);
        }
    }

    if(!printqueue->empty())        //checking whether the queue is empty
    {
        if(printqueue->front().blocked == 0)       //if the process at the front has completed its blocked time
        {
            proc = printqueue->front(); printqueue->pop();      //pop the process and push it on the unblocked process vector
            unblocked->push_back(proc);
        }
    }

    return;
}


void *inputQueue(void* args)            //for updating the queue with processes that are blocked with input, randBlock = 0
{
    PCB * proc = (PCB*)(args);
    inputqueue->push(*proc);        //pushing the proc inside input queue

    return NULL;
}


void *outputQueue(void* args)           //for updating the queue with processes that are blocked with output. randBlock = 1
{
    PCB * proc = (PCB*)(args);
    outputqueue->push(*proc);       //pushing proc inside output queue

    return NULL;
}


void *printQueue(void* args)         //for updating the queue with processes that are blocked with print, randBlock = 2
{
    PCB * proc = (PCB*)(args);
    printqueue->push(*proc);    //pushing the proc inside print queue

    return NULL;
}


int randQueue()        //randomly generates number to point the queue
{
    srand(time(NULL));
    int num = rand()%3;

    return num;
}