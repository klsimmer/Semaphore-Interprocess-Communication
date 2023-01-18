//
//  main.cpp
//  PA3
//
//  Created by Kaytlin Simmer on 12/2/21.
//
//this program works perfectly on my computer but core dumps on moodle for some reason :(

#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <semaphore.h>
#include <algorithm>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;


vector <long> accessEachDigit(long input)
{
    vector<long> digitVect;
    while (input > 0)
    {
        digitVect.push_back(input%10);
        input/=10;
    }
    reverse(digitVect.begin(), digitVect.end());
    return digitVect;
}

struct pTc
{
    int value;
    int threadNumber;
    sem_t *semA;
    vector <string> outputs;
};

struct cTg
{
    int value;
    int threadNumber;
    sem_t *semC;
    string output;
    pTc* child;
};

void *cTgFunc(void *y)
{
    /*
    //DEBUGGING
    cout<<endl;
    cout<<"CALLING GRANDCHILD PROCESS"<<endl;
    cout<<endl;
    //DEBUGGING END
     */
    
    cTg cthreadArg = *((cTg*) y);
    string s;
    s+= to_string(cthreadArg.value);
    s+=" = ";
    const int base = 10;
    const int segments = 7;
    const int segmentDisplay[base][segments] =
    {
      {1,1,1,1,1,1,0},
      {0,1,1,0,0,0,0},
      {1,1,0,1,1,0,1},
      {1,1,1,1,0,0,1},
      {0,1,1,0,0,1,1},
      {1,0,1,1,0,1,1},
      {1,0,1,1,1,1,1},
      {1,1,1,0,0,0,0},
      {1,1,1,1,1,1,1},
      {1,1,1,1,0,1,1}
    };
    for (int a = 0; a < segments; a++)
    {
        s+=to_string(segmentDisplay[cthreadArg.value][a]);
        if (a < segments)
            s+=" ";

    }
    
    cthreadArg.output = s;
    cthreadArg.child->outputs.push_back(s);
    
    /*
    //DEBUGGING
    cout<<endl;
    cout<<"GRANDCHILD PROCESS COMPLETED"<<endl;
    cout<<"OUTPUT VALUE: "<< s;
    cout<<endl;
    //DEBUGGING END
     */
    
    sem_post(cthreadArg.semC);
    return NULL;
}

void *pTcFunc(void *y)
{
    /*
    //DEBUGGING
    cout<<endl;
    cout<<"CALLING CHILD FUNCTION"<<endl;
    //DEBUGGING END
     */
    
    pTc threadArg = *((pTc*) y);
  //  *threadArg.ptrAddr = to_string(threadArg.value);
    sem_t *semD;
    const char name2[] = "SEMD";
    int init_value = 0;
    semD = sem_open(name2, O_CREAT,0666, init_value);
    
    vector<long> digitsOfVal = accessEachDigit(threadArg.value);
    const int ngcthreads = digitsOfVal.size();
    pthread_t gctid[ngcthreads];
    cTg arg;
    for (int b = 0; b < ngcthreads; b++)
    {
        arg.value = digitsOfVal.at(b);
        arg.threadNumber = b;
        arg.semC = semD;
        arg.child = &threadArg;
        /*
        //DEBUGGING
        cout<<endl;
        cout<<"Creating grandchild thread "<<i<<endl;
        cout<<"Value being passed: "<<digitsOfVal.at(i)<<endl;
        cout<<"Address of Semaphore: "<< semD <<endl;
        cout<<"Argument Address: "<< &arg<<endl;
        cout<<endl;
        //DEBUGGING END
         */
        
        if (pthread_create(&gctid[b], NULL, cTgFunc,&arg) != 0)
        {
            cerr<<"Error creating threads";
            return NULL;
        }
        sem_wait(semD);
    }
    
    
    for (int c = 0; c < ngcthreads; c++)
    {
        /*
        //DEBUGGING
        cout<<endl;
        cout<<"JOINING IN CHILD PROCESS"<<endl;
        cout<<endl;
        //DEBUGGING END
         */
        
        if (pthread_join(gctid[c], NULL))
        {
            cerr<<"Error joining threads";
            return NULL;
        }
        
        //print
        cout<<threadArg.outputs.at(c)<<endl;
    }
    
    sem_post(threadArg.semA);
    sem_close(semD);
    sem_unlink(name2);
    sem_destroy(semD);
    pthread_exit(NULL);
    
    return NULL;
}

//find digits in input to find number of gc threads needed
int digits(long inputs)
{
    int dig = 0;
    while (inputs != 0)
    {
        inputs = inputs/10;
        dig++;
    }
    return dig;
}

int main(int argc, char** argv)
{
    //semaphore
    sem_t *semB;
    const char name[] = "SEMB";
    int initial_value = 0;
    semB = sem_open(name, O_CREAT,0666, initial_value);

    //attain input
   long input;
   vector <long> values;
   while (cin>>input)
   {
       values.push_back(input);
   }
    /*
    //DEBUGGING
    cout<<"INPUT: "<<endl;
    for (int i = 0; i < values.size(); i++)
    {
        cout<<values.at(i)<<" ";
    }
    cout<<endl;
    //DEBUGGING END
     */
    
    
    const int nthreads = values.size();
    pthread_t tid[nthreads];
    pTc arg;

    for (int i = 0; i < nthreads; i++)
    {
        arg.value = values.at(i);
        arg.semA = semB;
        arg.threadNumber = i;
        
        /*
        //DEBUGGING
        cout<<endl;
        cout<<"Creating child thread "<<i<<endl;
        cout<<"Value being passed: "<<values.at(i)<<endl;
        cout<<"Address of Semaphore: "<< semB <<endl;
        cout<<"Argument Address: "<< &arg<<endl;
        cout<<endl;
        //DEBUGGING END
         */
        
        if (pthread_create(&tid[i], NULL, pTcFunc,&arg) != 0)
        {
            //fprintf(stderr,"Error creating threads\n");
            cerr<<"Error creating threads";
           // return 1;
        }
        sem_wait(semB);
        cout<<endl;
    }
    
    for (int j = 0; j < nthreads; j++)
    {
        if (pthread_join(tid[j],NULL) != 0)
        {
            cerr<<"Error joining threads";
            //return 2;
        }
    }

    sem_close(semB);
    sem_unlink(name);
    sem_destroy(semB);
    pthread_exit(NULL);
    
    return 0;
}
