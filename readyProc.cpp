#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sstream>
#include "PCB.h"
#include <sys/stat.h>
#include <queue>
#include <pthread.h>
#include <algorithm>
#include "comparator.h"
#include "flut.h"
using namespace std;


int toInt(char*);       //to convert the character array to the integer
bool strcmp(char*,string);      //compare the string with the character array
void sortPCB(PCB*, int);        //sort all the processes on the basis of their arrival
void swap(PCB*,PCB*);       //swap the values of two processes
void FCFSrun(queue<PCB>*, int*, int*, int*);            //contain the implementation of FCFS
void RRrun(queue<PCB>*, int, int*, int*, int*);         //contain the implementation of RR
void SJFrun(priority_queue<PCB,vector<PCB>,comparator>*, int*, int*, int*);     //contain the implementation of SJF
void SRTF(priority_queue<PCB,vector<PCB>,comparator>*, int*, int*, int*);       //contain the implementation of SRTF
PCB getMinimum(priority_queue<PCB,vector<PCB>,comparator>*, int);       //to get the process with minimum burst from min heap according to the clock
void* fromNewState(void*);      //getting data from pipe between new state and read state
void* fromBlockedState(void*);      //getting data from pipe between blocked and ready state
void* fromRunState(void*);        //getting data from pipe between ready and run state


int proc_Count, quantum, sigma = -1;        //stores the preliminary information regarding processes, algo and aid in their running
char algo[4];
PCB *procs;
queue<PCB>* readyProc_queue = new queue<PCB>;       //implementing the ready queue for FCFS and RR algo
priority_queue <PCB,vector<PCB>,comparator> *min_heap = new priority_queue <PCB,vector<PCB>,comparator>;    //implementing the min heap for SJF and SRTF


int main(int argc, char* args [])
{
    int pipe_fd_ready[2], pipe_fd_run[2], pipe_fd_new[2], pipe_fd_runBlock[2];      //store file descriptors of pipes
    pipe_fd_ready[0] = toInt(args[0]);      //ready state to running state pipe
    pipe_fd_ready[1] = toInt(args[1]);
    pipe_fd_run[0] = toInt(args[2]);        //running to ready state pipe
    pipe_fd_run[1] = toInt(args[3]);
    pipe_fd_new[0] = toInt(args[4]);        //new to ready state pipe
    pipe_fd_new[1] = toInt(args[5]);
    pipe_fd_runBlock[0] = toInt(args[6]);       //blocked to ready state pipe
    pipe_fd_runBlock[1] = toInt(args[7]);

    pthread_t thread_one, thread_two, thread_three;
    pthread_create(&thread_one,NULL,fromNewState,(void*)(&pipe_fd_new[0]));
    pthread_join(thread_one,NULL);
    

    sortPCB(procs,proc_Count);      //sorting the processes on the basis of their arrival

    if(strcmp(algo,"FCFS"))     //checks to determine the algorithm given
    {
        for(int a=0;a<proc_Count;a++)       //filling queue with processes
        {
            readyProc_queue->push(procs[a]);
        }

        FCFSrun(readyProc_queue,pipe_fd_ready,pipe_fd_run,pipe_fd_runBlock);
    }
    else if(strcmp(algo,"RR"))
    {
        for(int a=0;a<proc_Count;a++)       //filling queue with processes
        {
            readyProc_queue->push(procs[a]);
        }

        RRrun(readyProc_queue,quantum,pipe_fd_ready,pipe_fd_run,pipe_fd_runBlock);
    }
    else if(strcmp(algo,"SJF"))
    {
        for(int a=0;a<proc_Count;a++)       //filling min heap with processes
        {
            min_heap->push(procs[a]);
        }

        SJFrun(min_heap,pipe_fd_ready,pipe_fd_run,pipe_fd_runBlock);
    }
    else if(strcmp(algo,"SRTF"))
    {
        for(int a=0;a<proc_Count;a++)           //filling min heap with processes
        {
            min_heap->push(procs[a]);
        }

        SRTF(min_heap,pipe_fd_ready,pipe_fd_run,pipe_fd_runBlock);
    }


    return 0;
}


int toInt(char* arr)    //to convert the character array to the integer
{
    stringstream toint;     //buffer for storing the data
    toint<<arr;     //moving data from character array into the buffer
    int temp;
    toint>>temp;        //moving data from buffer into the integer
    return temp;
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


void FCFSrun(queue<PCB>* readyProc_queue, int *pipe_fd_ready, int *pipe_fd_run, int *pipe_fd_runBlock)      //contains FCFS implementation
{
    PCB* proc = new PCB;        //store the information of the process read or written
    int read_head = 0;      //stores the value returned by the read command
    int clock = 0;      //clock of the read side of the stimulator
    
    pthread_t thread_one, thread_two;       //for creating thread
    flut thread_one_data, thread_two_data;           //for passing the data in the thread
    thread_one_data.characters = algo;  thread_two_data.characters = algo;      //stores the algorithm
    thread_one_data.numbers = pipe_fd_runBlock[0];  thread_two_data.numbers = pipe_fd_run[0];         //stores the reading head of the pipe between blocked and ready state

    while(1)
    {
        if(sigma == -1)        //if signal is valid
        {
            if(!readyProc_queue->empty())   //sends data if queue isn't empty
            {
                if(readyProc_queue->front().arrival <= clock)       //if arrival has been occured
                {
                    sigma = 0;     //reset the signal
                    *proc = readyProc_queue->front();     readyProc_queue->pop();
                    write(pipe_fd_ready[1],proc,sizeof(PCB));       //pop process and write in the file
                }
            }
        }

        clock++;        //incrementing clock
        sleep(1);       //depicts one tick of time

        pthread_create(&thread_two,NULL,fromRunState,(void*)(&thread_two_data));    pthread_join(thread_two,NULL);      //creating thread for getting data from the running state and joining it with the main thread

        pthread_create(&thread_one,NULL,fromBlockedState,(void*)(&thread_one_data));    pthread_join(thread_one,NULL);      //creating thread for getting data from the blocked queue and joining it with the main thread
    }
    return;
}


void sortPCB(PCB* procs,int size)                 //sort the processes on the basis of the arrival time in ascending order
{
    for(int a=0;a<size;a++)
    {
        for(int b=0;b<size-a-1;b++)
        {
            if(procs[b].arrival > procs[b+1].arrival )          //sorts the processes based on the arrival time
            {
                swap(procs[b],procs[b+1]);
            }
        }
    }
    return;
}


void swap(PCB* proc1,PCB* proc2)           //swap values of the two processes
{
    //swaps the information of the processes into eachother
    int tempArr,tempBurst,tempWait,tempBlocked,tempID,tempCompletion, tempCompletedBurst;
    tempArr = proc1->arrival;   tempBurst = proc1->burst;   tempWait = proc1->waiting;  tempBlocked = proc1->blocked;
    tempID = proc1->trackID;    tempCompletion = proc1->completion; tempCompletedBurst = proc1->completedBurst;

    proc1->trackID = proc2->trackID;    proc1->arrival = proc2->arrival;    proc1->burst = proc2->burst;
    proc1->waiting = proc2->waiting;    proc1->blocked = proc2->blocked;    proc1->completedBurst = proc2->completedBurst;
    proc1->completion = proc2->completion;

    proc2->trackID = tempID;    proc2->arrival = tempArr;   proc2->burst = tempBurst;   proc2->waiting = tempWait;
    proc2->blocked = tempBlocked;   proc2->completedBurst = tempCompletedBurst;     proc2->completion = tempCompletion;

    return;
}


void RRrun(queue<PCB>* readyProc_queue, int quantum, int* pipe_fd_ready, int* pipe_fd_run, int* pipe_fd_runBlock)
{
    PCB* proc = new PCB;    //store the information of the process read or written
    int read_head = 0;      //stores the value returned by the read command
    int clock = 0;          //clock of the read side of the stimulator
    
    pthread_t thread_one, thread_two;       //for creating thread
    flut thread_one_data, thread_two_data;           //for passing the data in the thread
    thread_one_data.characters = algo;  thread_two_data.characters = algo;      //stores the algorithm
    thread_one_data.numbers = pipe_fd_runBlock[0];  thread_two_data.numbers = pipe_fd_run[0];         //stores the reading head of the pipe between blocked and ready state

    write(pipe_fd_ready[1],&quantum,sizeof(int));       //write the quantum value for the algo

    while(1)
    {
        if(sigma == -1)        //if signal is valid
        {
            if(!readyProc_queue->empty())       //sends data if queue isn't empty
            {
                sigma = 0;
                if(readyProc_queue->front().arrival <= clock)       //if arrival has been occured
                {
                    *proc = readyProc_queue->front();     readyProc_queue->pop();       //pops the process and dispatch it
                    write(pipe_fd_ready[1],proc,sizeof(PCB));
                }
                else
                {
                    sigma = -1;
                }
                
            }
        }

        clock++;    //incrementing clock
        sleep(1);       //depicts one tick of time

        pthread_create(&thread_two,NULL,fromRunState,(void*)(&thread_two_data));    pthread_join(thread_two,NULL);      //creating thread for getting data from the running state and joining it with the main thread

        pthread_create(&thread_one,NULL,fromBlockedState,(void*)(&thread_one_data));    pthread_join(thread_one,NULL);      //creating thread for getting data from the blocked queue and joining it with the main thread
    }

    return;
}


void SJFrun(priority_queue<PCB,vector<PCB>,comparator>* min_heap, int* pipe_fd_ready, int* pipe_fd_run, int* pipe_fd_runBlock)      //contaiins the implementation of the SJF
{
    PCB* proc = new PCB;        //store the information of the process read or written
    int read_head = 0;          //stores the value returned by the read command
    int clock = 0;      //clock of the read side of the stimulator
    
    pthread_t thread_one, thread_two;       //for creating thread
    flut thread_one_data, thread_two_data;           //for passing the data in the thread
    thread_one_data.characters = algo;  thread_two_data.characters = algo;      //stores the algorithm
    thread_one_data.numbers = pipe_fd_runBlock[0];  thread_two_data.numbers = pipe_fd_run[0];         //stores the reading head of the pipe between blocked and ready state

    while(1)
    {
        if(sigma == -1)        //if signal is valid
        {
            if(!min_heap->empty())      //if heap isn't empty
            {
                if(min_heap->top().arrival <= clock)        //if process has arrived
                {
                    sigma = 0;     //reset signal
                    *proc = min_heap->top(); min_heap->pop();
                    write(pipe_fd_ready[1],proc,sizeof(PCB));       //pop process and write in the file
                }
                else
                {
                    *proc = getMinimum(min_heap,clock);     //if the arrival has't reached by the smallest process, find the least process that has arrived
                    if(proc->trackID != 0)
                    {
                        sigma = 0;     //reset signal
                        write(pipe_fd_ready[1],proc,sizeof(PCB));   //distpatch the process
                    }
                }
                
            }
        }

        clock++;        //incrementing clock
        sleep(1);       //depicts one tick of time

        pthread_create(&thread_two,NULL,fromRunState,(void*)(&thread_two_data));    pthread_join(thread_two,NULL);      //creating thread for getting data from the running state and joining it with the main thread

        pthread_create(&thread_one,NULL,fromBlockedState,(void*)(&thread_one_data));    pthread_join(thread_one,NULL);      //creating thread for getting data from the blocked queue and joining it with the main thread
    }

    return;
}


PCB getMinimum(priority_queue<PCB,vector<PCB>,comparator> *min_heap, int clock)     //to get the process with minimum burst from min heap according to the clock
{
    size_t size = min_heap->size();
    vector<PCB> procs;      //for storing processes that doesn't follow the condition
    PCB proc;   //storing the process information that follows the condition
    int minBurst = INT32_MAX;       //for finding the process with minimum burst

    for(int a=0;a<size;a++)
    {
        if(min_heap->top().arrival <= clock)        //if process has arrived
        {
            if(min_heap->top().burst < minBurst)        //if process has minimum burst than the last one
            {
                if(proc.trackID != 0)
                {
                    PCB temp = proc;
                    proc = min_heap->top(); min_heap->pop();    procs.push_back(temp);      //stores the previous one in the vector
                    minBurst = proc.burst;
                }
                else
                {
                    proc = min_heap->top(); min_heap->pop();        //stores the process and pop it from min heap
                    minBurst = proc.burst;
                }
            }
        }
        else
        {
            procs.push_back(min_heap->top());   min_heap->pop();        //push the process to vector if it doesn't follow the condition
        }
    }

    size = procs.size();
    for(int a=0;a<size;a++)
    {
        min_heap->push(procs.back());   procs.pop_back();       //push the processes back to the min heap
    }

    return proc;
}


void SRTF(priority_queue<PCB,vector<PCB>,comparator>* min_heap, int* pipe_fd_ready, int* pipe_fd_run, int* pipe_fd_runBlock)        //contains the implementation of SRTF
{
    PCB* proc = new PCB;        //store the information of the process read or written
    int read_head = 0;          //stores the value returned by the read command
    int clock = 0;              //clock of the read side of the stimulator

    pthread_t thread_one, thread_two;       //for creating thread
    flut thread_one_data, thread_two_data;           //for passing the data in the thread
    thread_one_data.characters = algo;  thread_two_data.characters = algo;      //stores the algorithm
    thread_one_data.numbers = pipe_fd_runBlock[0];  thread_two_data.numbers = pipe_fd_run[0];         //stores the reading head of the pipe between blocked and ready state

    while(1)
    {
        if(sigma == -1)        //if signal is valid
        {
            if(!min_heap->empty())      //sends data if min heap isn't empty
            {
                if(min_heap->top().arrival <= clock)        //if arrival has been occured
                {
                    sigma = 0;     //reset signal
                    *proc = min_heap->top(); min_heap->pop();   //pops the process and dispatch it
                    write(pipe_fd_ready[1],proc,sizeof(PCB));
                }
                else
                {
                    *proc = getMinimum(min_heap,clock);     //if the arrival has't reached by the smallest process, find the least process that has arrived
                    if(proc->trackID != 0)
                    {
                        sigma = 0; //reset signal
                        write(pipe_fd_ready[1],proc,sizeof(PCB));       //distpatch the process
                    }
                }
            }
        }

        clock++;    //incrementing clock
        sleep(1);       //depicts one tick of time

        pthread_create(&thread_two,NULL,fromRunState,(void*)(&thread_two_data));    pthread_join(thread_two,NULL);      //creating thread for getting data from the running state and joining it with the main thread

        pthread_create(&thread_one,NULL,fromBlockedState,(void*)(&thread_one_data));    pthread_join(thread_one,NULL);      //creating thread for getting data from the blocked queue and joining it with the main thread

    }

    return;
}


void* fromNewState(void* args)          //getting data from pipe in the new state
{
    int *pipe_fd_new = (int*)(args);        //read end of the pipe from new state to ready state

    read(*pipe_fd_new,&proc_Count,sizeof(int));       //reading data from the new state
    read(*pipe_fd_new,&quantum,sizeof(int));
    read(*pipe_fd_new,algo,4);

    procs = new PCB [proc_Count];

    for(int a=0;a<proc_Count;a++)       //reading the proceses from the pipe
    {
        read(*pipe_fd_new,&procs[a],sizeof(PCB));
    }

    return NULL;
}


void* fromBlockedState(void* args)      //getting data from pipe between blocked and ready state
{
    flut *data = (flut*)args;       //the data sent to the thread

    int *size = new int(0);     //use for the numbers of processses in the blocked queue
    int read_head = 0;      //stores the value returned by the read command
    PCB *proc = new PCB;        //for storing the information of the process read

    if(strcmp(data->characters,"FCFS") || strcmp(data->characters,"RR"))        //checks for the spcifics algorithms
    {
        read_head = read(data->numbers,size,sizeof(int));       //reading the data from pipe if available
        if(read_head != -1)
        {
            for(int a=0;a<*size;a++)        //reading the processes from the pipe
            {
                read(data->numbers,proc,sizeof(PCB));
                readyProc_queue->push(*proc);       //pushing the processes in the ready queue
            }
        }
    }
    else if(strcmp(data->characters,"SJF") || strcmp(data->characters,"SRTF"))      //checks for the spcifics algorithms
    {
        read_head = read(data->numbers,size,sizeof(int));       //reading the data from pipe if available
        if(read_head != -1)
        {
            for(int a=0;a<*size;a++)        //reading the processes from the pipe
            {
                read(data->numbers,proc,sizeof(PCB));
                min_heap->push(*proc);      //pushing the processes in the min heap
            }
        }
    }
    
    return NULL;
}


void* fromRunState(void* args)      //getting data from pipe between running and ready state
{
    flut *data = (flut*)args;       //the data sent to the thread

    int temp = 0;
    int read_head = 0;
    PCB *proc = new PCB;

    if(strcmp(data->characters,"FCFS") || strcmp(data->characters,"RR"))        //checks for the spcifics algorithms
    {
        if(sigma == -1)        //check if the signal is valid
        {
            temp = sigma;
        }
        read_head = read(data->numbers,&sigma,sizeof(int));       //checking for signal from running state
        if(read_head != -1)     //if signal has been sent from running 
        {
            read_head = read(data->numbers,proc,sizeof(PCB));      //read for the data
            if(read_head != -1)
            {
                readyProc_queue->push(*proc);
            }
        }
        if(temp != sigma)      //if signal was valid before reading, 
        {
            sigma = -1;
        }
    }
    else if(strcmp(data->characters,"SJF") || strcmp(data->characters,"SRTF"))      //checks for the spcifics algorithms
    {
        if(sigma == -1)    //check if the signal is valid
        {
            temp = sigma;
        }
        read_head = read(data->numbers,&sigma,sizeof(int));
        if(read_head != -1)     //if running state has sent the signal start reading the processes from running pipe
        {
            read_head = read(data->numbers,proc,sizeof(PCB));
            if(read_head != -1)
            {
                min_heap->push(*proc);      //read from the pipe and push in min heap
            }
        }
        if(temp != sigma)      //if signal was valid before reading, make it valid again
        {
            sigma = -1;
        }
    }

    return NULL;
}