#ifndef PCB_H_
#define PCB_H_


#include <iostream>
#include <string>
using namespace std;


class PCB       //class that stores the information of the PCB
{
    public:

        int trackID;        //id of the process
        int arrival;            //arrival time of the process
        int waiting;            //waiting time of the process
        int burst;              //burst time of the process
        int completion;             //completion time of the process
        int blocked;                    //blocked time of the process
        int completedBurst;             //completed burst of the process

    PCB();
    PCB(int,int,int,int,int,int, int);
    void clear();

};


PCB::PCB()
{
    trackID=0;
    arrival=0; waiting=0; burst=0; completion=0, blocked=0, completedBurst=0;
}


PCB::PCB(int trackid,int arr,int wait,int bur,int comp, int block, int completed)
{
    trackID=trackid;
    arrival=arr; waiting=wait; burst=bur; completion=comp, blocked=block, completedBurst=completed;
}


void PCB::clear()       //clear the data stored in the object of the process
{
    trackID=0;
    arrival=0; waiting=0; burst=0; completion=0, blocked=0, completedBurst=0;

    return;
}


#endif