#include<iostream>
#include<unistd.h>
#include <sys/mman.h>

using namespace std;

#define MAX_PROCESSES 9
#define MAX_CHILDREN 2
 
int main()
{
    /// To keep count of the processes
    int* ProcessCount = (int*) mmap(NULL, sizeof (int) , PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *ProcessCount = 0;

    /// Flag to make only one process to create 9th process
    bool* lastProcess = (bool*) mmap(NULL, sizeof (bool) , PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *lastProcess = false;

    /// Keep track of pid of initial process
    pid_t* initialProcess = (pid_t*) mmap(NULL, sizeof (pid_t) , PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    /// To get parent for last process creation, as no process can create more than 2 processes
    pid_t* parentToLastProcess = (pid_t*) mmap(NULL, sizeof (pid_t) , PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
   

    cout << "Initial Process ID " << getpid() << ", from Parent " << getppid() << "\n";

    *initialProcess = getpid();
    
    for(int i = 0; i < MAX_CHILDREN; i++) /// loop will run MAX_CHILDREN times
    {
        if(fork() == 0)
        {
            *ProcessCount = *ProcessCount+1; /// Increasing count with each child creation
            cout << "[Child] pid " <<  getpid() << " from [Parent] pid " << getppid() << "\n";

            if(*ProcessCount >= MAX_PROCESSES)
                break; /// If expected process count is met then break the loop
            else
            {
                /// Each child process will create new children
                if(fork() == 0)
                {
                    *ProcessCount = *ProcessCount+1;
                    cout << "[Child] pid " <<  getpid() << " from [Parent] pid " << getppid() << "\n";
                }
                if(i == 1 && getpid()!=*initialProcess) {
                         *parentToLastProcess = getpid();
                }
            }
        }
        
    }
   
    /// In above code as in for loop, every time 2 processes will get created 
    /// (only even numbers of processes can be created)
    /// Thus, to create a last process (9th), below code is used
    /// As the initial process has already created 2 children, the last process should not be from intial process
    if(getpid() != *initialProcess && getpid() == *parentToLastProcess && !(*lastProcess))
    {
        *lastProcess = true;
        if(*ProcessCount < MAX_PROCESSES && fork() == 0)
        {
            *ProcessCount = *ProcessCount+1;
            cout << "[Child] pid " <<  getpid() << " from [Parent] pid " << getppid() << "\n";
        }
    }

    /// Wait for all child processes to terminate before exiting
    int status;
    while (wait(&status) > 0)

    munmap(ProcessCount, sizeof(int));
    munmap(lastProcess, sizeof(bool));
    munmap(initialProcess, sizeof(pid_t));
    munmap(parentToLastProcess, sizeof(pid_t));

    return 0;
}