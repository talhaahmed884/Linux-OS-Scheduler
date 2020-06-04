#ifndef FLUT_H_
#define FLUT_H_


#include <iostream>
using namespace std;


class flut      //class for passing data to the threads
{
    public:
        
        char *characters;           //stores the characters
        int numbers;                    //stores the data in numbers

    flut();

};


flut::flut()
{
    characters = NULL;
    numbers = 0;
}


#endif