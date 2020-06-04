#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sstream>
#include "PCB.h"
#include <sys/stat.h>
#include <list>
#include <iterator>
using namespace std;


int toInt(char*);       //to convert the character array to the integer
char* toChar(string);       //to convert the string in to a character array
void write(PCB);        //write the process information into the file "processes_stats.txt"
void write(list<PCB>*);         //write the average values of the processes exited so far
void printGanttChart(PCB);      //prints the gantt chart on display


int main(int args, char* argc[])
{
    int pipe_fd_exit[2];        //pipes for connection between running state and exit state
    pipe_fd_exit[0] = toInt(argc[0]);       //read head of the pipe between running and exit state
    pipe_fd_exit[1] = toInt(argc[1]);       //write head of the pipe between running and exit state

    PCB* proc = new PCB;        //for storing the information of the incoming process
    list<PCB>* exit_proc = new list<PCB>;       //list that stores the information of all the processes exited
    int cpu_clock = 0;      //cpu clock that runs and assign each exiting process its completion time

    while(1)
    {
        int read_head = read(pipe_fd_exit[0],proc,sizeof(PCB));     //checks if any data is available to be read
        if(read_head != -1)
        {
            proc->completion=cpu_clock;     //assign the ending process its completion
            proc->burst = proc->completedBurst;     //assign the original value to the burst
            proc->waiting = proc->completion-proc->arrival-proc->burst;     //calculate the waiting time at the end by using a very simple formula
            write(*proc);   exit_proc->push_back(*proc);        //write the process in the file and store it in the list containing all the exited processes
            printGanttChart(*proc);
        }

        sleep(1);       //depicts one tick of time
        cpu_clock++;        //cpu clock counter

        if(cpu_clock%30 == 0)       //write the average process data into the file
        {
            write(exit_proc);       //functon for writing average data into the file
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


char* toChar(string str)           //converts the string into the character array
{
    char *ch = new char[str.length()];      //initializing the character array of the size of the string
    for(int a=0;str[a]!='\0';a++)           //storing it into the character array
    {
        ch[a]=str[a];
    }
    return ch;
}


void write(PCB proc)            //writes the process into the file processes_stats.txt
{
    int file_write = open("processes_stats.txt",O_WRONLY|O_APPEND);     //opening the file for appending
    stringstream str;       //intializing a buffer

    str<<"Proc";        //storing the stats of the process in a specific format in a buffer
    str<<proc.trackID;
    str<<"\n";
    str<<"Arrival: ";
    str<<proc.arrival;
    str<<"\n";
    str<<"Burst: ";
    str<<proc.burst;
    str<<"\n";
    str<<"Waiting: ";
    str<<proc.waiting;
    str<<"\n";
    str<<"Turn Around Time: ";
    str<<(proc.completion-proc.arrival);
    str<<"\n";
    str<<"Completion: ";
    str<<proc.completion;
    str<<"\n\n";
    
    string data = str.str();        //converting it into a string
    char *buff = toChar(data);      //converting it into a character array
    write(file_write,buff,data.length());           //writing the character array into the file

    close(file_write);          //closing the file
    return;
}


void write(list<PCB>* exit_proc)        //write the average values of the processes exited so far
{
    size_t size = exit_proc->size();        //getting the size of the list containing exited processes
    list<PCB>::iterator lptr = exit_proc->begin();      //defining the iterator

    int tat = 0, waiting = 0, throughput = 0;       //the storage area for the average values of all processes

    for(int a=0;a<size;a++)     //loop until the size of the list
    {
        tat += (lptr->completion - lptr->arrival);      //add the values to their respective storage area
        waiting += lptr->waiting;
        throughput += lptr->completedBurst;
        lptr++;
    }

    if(size > 0)
    {
        tat /= size;    waiting /= size;    throughput/=size;       //calculate the average values
    }

    int file_write = open("processes_stats.txt",O_WRONLY|O_APPEND);     //opening the file for appending
    stringstream str;       //intializing a buffer

    str<<"Throughput: "<<throughput<<endl;      //storing the data in the buffer
    str<<"Average Turn Around Time: "<<tat<<endl;
    str<<"Average Waiting Time: "<<waiting<<endl<<endl;

    string data = str.str();        //converting it into a string
    char *buff = toChar(data);      //converting it into a character array

    write(file_write,buff,data.length());           //writing the character array into the file

    close(file_write);          //closing the file
    return;
}


void printGanttChart(PCB proc)      //prints the gantt chart on display
{
    cout<<"Process "<<proc.trackID<<" has completed its execution.\n";
    cout<<"Arrival      Burst      Waiting      Completion      Turn Around Time\n";
    cout<<proc.arrival<<"            "<<proc.completedBurst<<"          "<<proc.waiting<<"            "<<proc.completion<<"               "<<proc.completion-proc.arrival<<endl<<endl;
    return;
}