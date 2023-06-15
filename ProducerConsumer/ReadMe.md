                                    COEN 283 Assignment 2

    STUDENT INFO 
    -----------------------------------------------
    Student Name : Kartiki Rajendra Dindorkar
    Student ID : W1651519
    Email ID : kdindorkar@scu.edu

    Steps To Execute
    -----------------------------------------------
    1. Unzip the given folder "W1651519"
    2. Open the folder and run make file using command : make
    3. It will create an executable file "prodcon"
    4. Execute file using command : ./prodcon <NoOfConsumerThreads> <MaxNumberToBeGenerated>
    5. Example : ./prodcon 3 6
    6. Here, the second argument <MaxNumberToBeGenerated> should be greater than 5, as the buffer size is 5 and to test circular buffer, number of inputs should be greater than 5
    7. If you executed only by running "./prodcon" (without any arguments) file. Then it will take following values as default :
        a. num_consumers = 2; /// Specify on cmd line; default is two. 
        b. max_number = 10; /// Specify on cmd line; default is ten
   
    Results
    -------
    1. Above execution will result in :

    kartikidindorkar@Kartikis-MacBook-Pro W1651519 % make

    g++ -std=c++11 -Wall -Wextra -pthread -c -o prodcon.o prodcon.cpp
    g++ -std=c++11 -Wall -Wextra -pthread -o prodcon prodcon.o

    kartikidindorkar@Kartikis-MacBook-Pro W1651519 % ./prodcon 3 6

    Producer wrote 0
    Producer wrote 1
    Consumer 1809510400, read 0
    Consumer 1809510400, read 1
    Producer wrote 2
    Consumer 1810083840, read 2
    Producer wrote 3
    Consumer 1809510400, read 3
    Producer wrote 4
    Consumer 1810083840, read 4
    Producer wrote 5
    Consumer 1810657280, read 5

    2. Here, the producer thread will generate 6 numbers (as per the 2nd argument passed through cmd line) 0 to 5.
    3. producer thread will write these numbers onto circular buffer.
    4. Consumer Threads will read these numbers and print them onto the cosnole.
    5. To re-compile the file first run : make clean (to remove all the executables generated by previous make)
    6. Please refer following images from the folder for outputs :
        a. Output_1 : Running the program using make and arguments
        b. Output_default : Executing the program using previously generated executable and without any arguments (default arguments will be taken as 2 & 10)
        c. Output_makeClean : Use of make clean to delete previous executable and run again


    Discussion
    ----------

    1. How many of each integer should you see printed?
    Ans : This program will print the numbers from 0 to (<MaxNumberToBeGenerated> - 1)

    2. In what order should you expect to see them printed? Why?
    Ans : 
    i. As the buffer size is 5, I expected that producer will write first 5 integers and then consumer will consume them.
    ii. Similarly this will happen for next 5 numbers.
    iii. Thus, I expected that the numbers will be produced in the batch of 5 and consumed in similar fashion


    3. Did your results differ from your answers in (1) and (2)? Why or why not?
    Ans :
    i. As we are using semaphores and condition variables, once producer produces an integer, it leaves the critical section guarded by a mutex lock and broadcasts that a number is available for consumptio.
    ii. Simultaneoulsy, consumer thread was waiting for signal and once producer sends the signal, Consumer goes and try to acquire lock on Circular buffer inorder to read the data.
    iii. Thease are not atomic operations, thus whichever thread finishes it's work, waits for critical section to get unlocked.
    iv. Thus, the result will be first producer will generate some numbers and once consumer gets signal and it's wait for critical section is over, it will read those numbers.
    v. This switching will be done randomly. But consumer will consume only after producer has produced those intergers.

