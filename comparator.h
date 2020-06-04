#ifndef COMPARATOR_H_
#define COMPARATOR_H_


#include <iostream>
#include "PCB.h"
using namespace std;


class comparator            //class for providing the compare function for creating min heap
{
    public:
        int operator() (const PCB&, const PCB&);        //extending the built in compare operator for the specified class

};


int comparator::operator() (const PCB& p1, const PCB& p2)       //actual implementation of the operator extended
{
    return p1.burst > p2.burst;
}


#endif