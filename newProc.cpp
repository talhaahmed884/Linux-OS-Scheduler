#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sstream>
#include "PCB.h"
#include <sys/stat.h>
#include <pthread.h>
using namespace std;


void init(char*, int);      //allocate buffer with null values to remove all the garbage data if any
int toInt(char*);       //to convert the character array to the integer
int countProc(char*);       //count the number of processes in the file 'processes.txt'
int toInt(string);          //to convert the string to the integer
string getData(char*,int*);         //to get the data from the file with the given index
char* toChar(string);       //to convert the string in to a character array
bool strcmp(char*,string);      //compare the string with the character array
char* toChar(int);          //to convert the integer in to a character array


int main(int argc, char* args [])
{
    char buff[5000];        //buffer for storing the data from the file
    init(buff,5000);        //initialize the buffer with null

    int file_fd = open("processes.txt",O_RDWR);     //open the file for reading
    read(file_fd,&buff,sizeof(buff));       //reading the data from the file
    
    int quantum,proc_Count;     //storage area for storing quantum, number of processes, algo
    char algo[4];
    int index = 0;
    string temp = getData(buff,&index);     //get the algo to be executed
    for(int a=0;temp[a]!='\0';a++)
    {
        algo[a]=temp[a];
    }
    if(strcmp(algo,"RR"))       //comparing the algo with the one having extra chunk of data
    {
        string input = getData(buff,&index);        //getting the quantum value from the file for the certain processes
        quantum=toInt(input);       //converting into the intger
    }
    else
    {
        quantum=0;          //if there are other algos, the quantum is set to zero
    }
    int proc_count = 0;
    proc_Count = countProc(buff);       //count the number of processes


    int pipe_fd[2];     //pipes for communication between new state and ready state
    pipe_fd[0] = toInt(args[0]);        //reading head of the pipe between new and ready state
    pipe_fd[1] = toInt(args[1]);        //writing head of the pipe between new and ready state

    write(pipe_fd[1],algo,sizeof(algo));        //writing the algo information into the pipe
    write(pipe_fd[1],&proc_Count,sizeof(int));      //writing the number of processes
    write(pipe_fd[1],&quantum,sizeof(int));     //writing the quantum value if any
    write(pipe_fd[1],algo,sizeof(algo));

    PCB *procs = new PCB[proc_Count];       //for storing the information of the processes from buffer

    for(int a=0;a<proc_Count;a++)
    {
        getData(buff,&index);
        procs[a].trackID = a+1;     //assigning the process id
        procs[a].arrival = toInt(getData(buff,&index));     //getting the arrival time
        procs[a].burst = toInt(getData(buff,&index));       //getting the burst time

        write(pipe_fd[1],&procs[a],sizeof(PCB));        //writing the data in the pipe
    }


    return 0;
}


void init(char* buff, int size)  //allocate buffer with null values to remove all the garbage data if any
{
    for(int a=0;a<5000;a++)
    {
        buff[a]='\0';       //at each index store null
    }
    return;
}


int toInt(char* arr)        //to convert the character array to the integer
{
    stringstream toint;     //buffer for storing the data
    toint<<arr;     //to store the character array into the buffer
    int temp;
    toint>>temp;        //storing the buffer data into the integer
    return temp;
}


int countProc(char* buff)       //count the number of processes in the file 'processes.txt'
{
    int count=0;
    for(int a=0;buff[a]!='\0';a++)
    {
        if(buff[a]=='p' || buff[a]=='P')        //count on the basis of the alphabet P in the file
        {
            count++;
        }
    }
    return count;
}


int toInt(string str)       //to convert the string to the integer
{
    stringstream toint;     //buffer for storing the data
    toint<<str;     //storing string into the buffer
    int temp;
    toint>>temp;        //storing buffer into the integer
    return temp;
}


string getData(char* buff,int* index)       //to get the data from the file with the given index
{
    string input;       //storage for storing the data from buffer
    for(*index;buff[*index]!='\n';++(*index))       //run from value of index till the next line
    {
        if(buff[*index]!='\0')
        {
            input += buff[*index];      //stores the data in the string
        }
        else
        {
            break;
        }
        
    }
    (*index)++;
    return input;
}


char* toChar(string str)        //to convert the string in to a character array
{
    char *ch = new char[str.length()];      //creates the character array of size of the string
    for(int a=0;str[a]!='\0';a++)
    {
        ch[a]=str[a];       //stores the string into character at each index
    }
    return ch;
}


bool strcmp(char* str1, string str2)        //compare the string with the character array
{
    int a=0,b=0;
    for(; str1[a]!='\0' || str2[b]!='\0'; a++,b++)      //runs until any of the variable size is over
    {
        if(str1[a]!=str2[b])
        {
            return false;
        }
    }
    if(a!=b)        //if size isn't equal, the variables doesn't match
    {
        return false;
    }
    else
    {
        return true;
    }
    

}


char* toChar(int var)       //to convert the integer in to a character array
{
    stringstream tochar;        //buffer to store data
    tochar<<var;        //store the integer into the buffer
    char *temp = new char[tochar.str().size()];     //creates the character array of size of the buffer
    for(int a=0;a<tochar.str().size();a++)
    {
        temp[a]=tochar.str()[a];        //stores the data into the character array
    }
    return temp;
}