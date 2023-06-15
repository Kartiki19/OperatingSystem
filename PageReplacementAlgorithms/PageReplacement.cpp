
#include <vector>
#include <list>
#include <random>
#include <algorithm>
#include <ctime>
#include <iostream>
#include <map>
#include <queue>
#include <unistd.h>
#include <numeric>
#include<iomanip>
#include <chrono>
#include <thread>
#include <queue>
using namespace std;

/// Constants
const int MEMORY_SIZE = 100; /// Memory size in MB
const int PAGE_SIZE = 1; /// Page size in MB
const int PAGES_IN_MEMORY = MEMORY_SIZE / PAGE_SIZE;  /// Calculate number of pages in memory
const int NUM_OF_PROCESSES = 150; /// Total number of processes 
const int TOTAL_RUNS = 5; /// Total Runs required

// Memory page struct to track page information
struct MemoryPage {
    bool occupied;
    int processId;
    int pageNumber;

    MemoryPage(bool occupied, int processId, int pageNumber) : occupied(occupied), processId(processId), pageNumber(pageNumber){}
};

/// Page Struct to store page information 
struct Page{
    int pageNumber;
    bool inMemory;
    int memoryLocation;
    int pID;
};

/// Process struct to store process information
struct Process {
    string name;
    int pID;
    int size;  // Size in pages
    int arrivalTime;  // Arrival time in seconds
    int serviceDuration;  // Service duration in seconds
    vector<Page> pT;
    static int count;

    Process(){}
    Process(string name, int id, int size, int arrival, int serviceDuration) : 
    name(name), pID(id), size(size), arrivalTime(arrival), serviceDuration(serviceDuration), pT(size)
    {
        count = count += size;
       // std::cout << "Size is : " <<  size << " count is : " << count << endl;
        for (int i = 0; i < size; ++i) {
            pT[i] = {i, false, -1, id};
        }   
    }
};
int Process::count = 0; /// Definition of static variable

// Generate random integer within a range [min, max]
int generateRandomNumber(int min, int max) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<> dis(min, max-1);
    return dis(gen);
}

class FreeMemory
{
    public:
        struct FreeSpace {
            int memoryLocation;
            FreeSpace* next;
            FreeSpace(int memoryLocation) : memoryLocation(memoryLocation){}
        };

        FreeSpace* head;
        FreeSpace* tail;

        FreeMemory() : head(nullptr), tail(nullptr) {}

        void addFreeMemory(int memoryLocation) {
            
            FreeSpace* newSpace = new FreeSpace(memoryLocation);
            newSpace->next = nullptr;

            if (tail == nullptr) {
                head = newSpace;
                tail = newSpace;
            } else {
                tail->next = newSpace;
                tail = newSpace;
            }
        }

        int deleteFreeMemory() {
            if (head == nullptr)
                throw runtime_error("Job queue is empty.");

            FreeSpace* spaceToRemove = head;
            int memoryLocation = spaceToRemove->memoryLocation;

            head = head->next;
            if (head == nullptr)
                tail = nullptr;

            delete spaceToRemove;
            return memoryLocation;
        }

        bool isEmpty() const {
            return head == nullptr;
        }
};
// JobQueue class to store the jobs in a linked list
class JobQueue 
{
    public:
        struct Job {
            int processId;
            int pageId;
            Job* next;
            Job(){}
            Job(int processId, int pageId)
            {
                this->processId = processId;
                this->pageId = pageId;
            } 
        };

        Job* head;
        Job* tail;

        JobQueue() : head(nullptr), tail(nullptr) {}

        void printLinkedList(map<int, Process>& ProcessList) 
        {
            Job* current = head;

            while (current != nullptr) {
                std::cout << "\nProcess " << current->processId
                    << " Page " << current->pageId 
                    << " In Memory " << ProcessList[current->processId].pT[current->pageId].inMemory 
                    << " Memory Location " << ProcessList[current->processId].pT[current->pageId].memoryLocation
                    << endl;
                current = current->next;
            }

            std::cout << endl;
        }

        void enqueue(int process, int page) {
            
            Job* newJob = new Job(process, page);
            newJob->next = nullptr;

            if (tail == nullptr) {
                head = newJob;
                tail = newJob;
            } else {
                tail->next = newJob;
                tail = newJob;
            }
        }

        pair<int, int> dequeue() {
            if (head == nullptr)
                throw runtime_error("Job queue is empty.");

            Job* jobToRemove = head;
            int processId = jobToRemove->processId;
            int pageId = jobToRemove->pageId;

            head = head->next;
            if (head == nullptr)
                tail = nullptr;

            delete jobToRemove;
            return  make_pair(processId, pageId);;
        }

        bool isEmpty() const {
            return head == nullptr;
        }
};

// Generate random process using given sizes and durations
void generateProcesses(map<int, Process>& ProcessList) 
{
    for (int i = 0; i < NUM_OF_PROCESSES; i++) 
    {
        /// Define the possible process sizes, service durations and number of processes to generate
        vector<int> sizes = {5, 11, 17, 31}; // in mb
        vector<int> service_durations = {1, 2, 3, 4, 5}; // in sec

        /// Initialize the random number generator with a seed value
        mt19937_64 selectRandomly(std::random_device{}());

        /// Generate Simulated Processes
        string name = "Process_" + to_string(i);
        int size = sizes[selectRandomly() % sizes.size()];
        int arrival = i*2;
        int duration = service_durations[selectRandomly() % service_durations.size()];
        ProcessList[i] = Process(name, i, size, arrival, duration);
    }
}
 
/// Driver function to sort the Processes by their arrival time
bool sortByAT(const Process& a, const Process& b)
{
    return (a.arrivalTime < b.arrivalTime);
}

void printVector(vector<Process>& processList)
{
  for (const auto& item : processList) {
        std::cout << "Name " << item.name << " AT " << item.arrivalTime << " PID " << item.pID << " Size " << item.size << " " <<endl;

        for(const auto&i : item.pT) {
            std::cout << "Page " << i.pageNumber << " In Memory " << i.inMemory << endl;
        }
    }
}

void printMap(map<int, Process>& processList)
{
    for (const auto& pair : processList) {
        std::cout << "Key " << pair.first << ", PID " << pair.second.pID << " Name " << pair.second.name << " AT " << pair.second.arrivalTime 
        << " Size " << pair.second.size <<endl;

         for(const auto&i : pair.second.pT) {
            std::cout << "Page " << i.pageNumber << " In Memory " << i.inMemory << " Memory Location " << i.memoryLocation << endl;
        }
    }
}

void fillMemory(vector<MemoryPage>& memoryMap, map<int, Process>& ProcessList, JobQueue& jobQueue)
{
    int memoryloc = 0;
    const int first25Process = 25;
    const int first4Pages = 4;
    for(int i = 0; i < NUM_OF_PROCESSES; i++){
        for(int j = 0; j < static_cast<int>(ProcessList[i].pT.size()); j++){
            if((i < first25Process) && (j < first4Pages) && memoryloc < PAGES_IN_MEMORY){
                memoryMap[memoryloc].occupied = true;
                memoryMap[memoryloc].processId = i;
                memoryMap[memoryloc].pageNumber = j;
                ProcessList[i].pT[j].inMemory = true;
                ProcessList[i].pT[j].memoryLocation = memoryloc;
                memoryloc++;
            }
            else{
                jobQueue.enqueue(i, j);
            }
        }
    }
}
// Print the queue
void printQueue(queue<int> q)
{
    queue<int> queue = q;
    while (!queue.empty()) {
        std::cout << '\t' << queue.front();
        queue.pop();
    }
    std::cout << '\n';
}

void printMemoryMap(vector<MemoryPage>& memoryMap){
    std::cout << "Memory Map : [ ";
    for(int i = 0; i < PAGES_IN_MEMORY; i++){
        string processID = ".";
        if(memoryMap[i].occupied) processID = to_string(memoryMap[i].processId);
        std::cout << processID << ","; 
    }
    std::cout << " ] " << endl;
}

void RemoveOldProcess(vector<MemoryPage>& memoryMap, int firstIn, FreeMemory& freeMemory, map<int, Process>& ProcessList)
{
    int oldProcessID = memoryMap[firstIn].processId;
    int oldPageID = memoryMap[firstIn].pageNumber;

    /// Free memory from Memory map
    memoryMap[firstIn].occupied = false;
    memoryMap[firstIn].processId = -1;
    memoryMap[firstIn].pageNumber = -1;

    /// Update Process list 
    ProcessList[oldProcessID].pT[oldPageID].inMemory = false;
    ProcessList[oldProcessID].pT[oldPageID].memoryLocation = -1;

    /// Add free memory to the linkedlist
    freeMemory.addFreeMemory(firstIn);
}

int AddNewProcess(vector<MemoryPage>& memoryMap, int processID, int pageID, FreeMemory& freeMemory, map<int, Process>& ProcessList)
{
    /// Take first free location from linked list
    int freeLocation = freeMemory.deleteFreeMemory();

    /// Add new process to free location in memory map
    memoryMap[freeLocation].occupied = true;
    memoryMap[freeLocation].processId = processID;
    memoryMap[freeLocation].pageNumber = pageID;

    /// Update the Process List
    ProcessList[processID].pT[pageID].inMemory = true;
    ProcessList[processID].pT[pageID].memoryLocation = freeLocation;
    return freeLocation;
}

void removeCompletedProcess(map<int, Process> &ProcessList, vector<MemoryPage> &memoryMap, FreeMemory &freeMemory, int newProcessID)
{
    usleep(10000);                                          /// Sleep for 10 millisecond to finish process and remove the process from the memory
    for (int j = 0; j < ProcessList[newProcessID].size; j++) /// Take out the completed process and add it as a free space
    {
        if (ProcessList[newProcessID].pT[j].inMemory)
        {
            int pageLocationInMemory = ProcessList[newProcessID].pT[j].memoryLocation;
            RemoveOldProcess(memoryMap, pageLocationInMemory, freeMemory, ProcessList);
        }
    }
    auto timenow = chrono::system_clock::to_time_t(chrono::system_clock::now());
    std::cout << "\n*EXIT : Current timestamp: " << ctime(&timenow) << " Process " << newProcessID << " EXITING, Size " << ProcessList[newProcessID].size
              << " pages, Service Duration " << ProcessList[newProcessID].serviceDuration << " sec" << endl;
}

class LRUCache {
public:
    class node{
        public:
        int key;
        node* prev = NULL;
        node* next = NULL;
        node(int _key)
        {
            key = _key;
        }
    };

    int cap = 0;
    node* head = new node(0);
    node* tail = new node(0);
    unordered_map<int, node*> lru;

    LRUCache(int capacity) {
        cap = capacity;
        head->next = tail;
        tail->prev = head;
    }
    
    void get(int key) {
        if(lru.find(key) == lru.end()){ std::cout << "\nNot in Memory";}
        else{
            node* current = lru[key];
            deletenode(current);
            addnode(current);
        }
    }
    
    void put(int key) {
        if(lru.find(key) != lru.end()){
            deletenode(lru[key]);
            lru.erase(key);
        }
        if(cap == static_cast<int>(lru.size())){
            lru.erase(tail->prev->key);
            deletenode(tail->prev);
        }
        node* newnode = new node(key);
        addnode(newnode);
        lru.insert({key, newnode});
    }

    int gettail()
    {
        node* tail_ = tail->prev;
        int key = tail_->key;
        deletenode(tail_);
        return(key);
    }

    void addnode(node *newnode){
        node* current = head->next;
        head->next = newnode;
        current->prev = newnode;
        newnode->prev = head;
        newnode->next = current;
    }

    void deletenode(node *delnode){
        node* delnext = delnode->next;
        node* delprev = delnode->prev;
        delprev->next = delnext;
        delnext->prev = delprev;
    }
};


int simulateFIFO(map<int, Process>& ProcessList, vector<MemoryPage>& memoryMap, JobQueue& jobQueue,  FreeMemory& freeMemory, int& totalHits, int& totalMisses, bool reference)
{
    int swappedInCount = 0;
    int requiredReferenceCount = Process::count;
    int requestCount = 0;
    if(reference) requiredReferenceCount = 100;
    auto start = std::chrono::steady_clock::now(); /// Get the start time

    queue<int> FIFO;
    for(int i = 0; i < static_cast<int>(memoryMap.size()); i++)
        FIFO.push(i); /// Filled que for already inserted processes
    
    while (!jobQueue.isEmpty() && requestCount < requiredReferenceCount) 
    {
        pair<int, int> currentJob = jobQueue.dequeue();
        int newProcessID = currentJob.first;
        int newPageID = currentJob.second;
        requestCount++;

        /* Printing Data for each memory reference : timestamp, process name, page reference, page-in-memory, which process/ page will be evicted */
        auto timenow = chrono::system_clock::to_time_t(chrono::system_clock::now());
        std::cout << "\n*NEW REQUEST = Current timestamp: " << ctime(&timenow) << " Process " << newProcessID << ", Page " << newPageID << ", In Memory " << ProcessList[newProcessID].pT[newPageID].inMemory;

        if(ProcessList.find(newProcessID) != ProcessList.end())
        {
            if(ProcessList[newProcessID].pT[newPageID].inMemory) totalHits++;
            else{ 
                totalMisses++; /// Not in memory

                /// Locality of reference
                for(int i = newPageID-1; i <= newPageID+1; i++){
                    if(i != -1 && i < ProcessList[newProcessID].size && !ProcessList[newProcessID].pT[i].inMemory)
                    {
                        if(freeMemory.head == nullptr && !FIFO.empty()){ /// Take out the first in process and add it as a free space
                            int firstIn = FIFO.front();
                            FIFO.pop();
                             std::cout << " Locality Of Reference = Page " << i << ", In Memory " << ProcessList[newProcessID].pT[i].inMemory;
                            std::cout << " Evicted Process " << memoryMap[firstIn].processId << " Page " << memoryMap[firstIn].pageNumber << endl;
                            RemoveOldProcess(memoryMap, firstIn, freeMemory, ProcessList);
                        }
                        else{
                            std::cout << " Locality Of Reference = Page " << i << ", In Memory " << ProcessList[newProcessID].pT[i].inMemory;
                            std::cout << " Free Space Available, No Eviction Needed !"<< endl;
                        }
        
                        if(freeMemory.head != nullptr){  /// Allocate the free space to new reference   
                            int filledLocation = AddNewProcess(memoryMap, newProcessID, i, freeMemory, ProcessList);
                            FIFO.push(filledLocation);
                            
                            if(i == 0){ /// Check whether process is just started
                                printMemoryMap(memoryMap);
                                swappedInCount++;
                                auto timenow = chrono::system_clock::to_time_t(chrono::system_clock::now());
                                std::cout << "*ENTER : Current timestamp: " << ctime(&timenow) << " Process " << newProcessID << " ENTERING, Size " << ProcessList[newProcessID].size 
                                          << " pages, Service Duration " << ProcessList[newProcessID].serviceDuration << " sec" << endl;
                            }
                        }
                        if (i >= ProcessList[newProcessID].size - 1){ /// Remove the process if last page of the process has been referenced (waiting for 10 ml before removing)
                            removeCompletedProcess(ProcessList, memoryMap, freeMemory, newProcessID);
                            printMemoryMap(memoryMap);
                        }
                    }
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto endTime = std::chrono::steady_clock::now(); // Get the end time
        auto durationInSec = std::chrono::duration_cast<std::chrono::seconds>(endTime - start);
        std::cout << "\nPages Referred : " << requestCount << endl;
        if(!reference && durationInSec > std::chrono::seconds(60)) /// Running simulation only for 1 min.
            break;
    }
    return (swappedInCount+25); /// 25 process were pre-filled
}

int simulateLRU(map<int, Process>& ProcessList, vector<MemoryPage>& memoryMap, JobQueue& jobQueue,  FreeMemory& freeMemory, int& totalHits, int& totalMisses, bool reference)
{
    int swappedInCount = 0;  

    int requiredReferenceCount = Process::count;
    int requestCount = 0;
    if(reference) requiredReferenceCount = 100;

    auto start = std::chrono::steady_clock::now(); /// Get the start time

    LRUCache LRU(100);

    for(int i = 0; i < static_cast<int>(memoryMap.size()); i++)
        LRU.put(i);

    while (!jobQueue.isEmpty() && requestCount < requiredReferenceCount) 
    {
        pair<int, int> currentJob = jobQueue.dequeue();
        int newProcessID = currentJob.first;
        int newPageID = currentJob.second;
        requestCount++;

        /* Printing Data for each memory reference : timestamp, process name, page reference, page-in-memory, which process/ page will be evicted */
        auto timenow = chrono::system_clock::to_time_t(chrono::system_clock::now());
        std::cout << "\n*NEW REQUEST = Current timestamp: " << ctime(&timenow) << " Process " << newProcessID << ", Page " << newPageID << ", In Memory " << ProcessList[newProcessID].pT[newPageID].inMemory << endl;

        if(ProcessList.find(newProcessID) != ProcessList.end())
        {
            if(ProcessList[newProcessID].pT[newPageID].inMemory){
                totalHits++;
                LRU.get(ProcessList[newProcessID].pT[newPageID].memoryLocation);
            }
            else{
                totalMisses++; /// Not in memory

                /// Locality of reference
                for(int i = newPageID-1; i <= newPageID+1; i++){
                    if(i != -1 && i < ProcessList[newProcessID].size && !ProcessList[newProcessID].pT[i].inMemory)
                    {
                        if(freeMemory.head == nullptr && LRU.tail->prev != nullptr){ /// Take out the first in process and add it as a free space
                            int leastRecent = LRU.gettail();
                            std::cout << "Locality Of Reference = Page " << i << ", In Memory " << ProcessList[newProcessID].pT[i].inMemory;
                            std::cout << " Evicted Process " << memoryMap[leastRecent].processId << " Page " << memoryMap[leastRecent].pageNumber << endl;
                            RemoveOldProcess(memoryMap, leastRecent, freeMemory, ProcessList);
                        }
                        else{
                             std::cout << "Locality Of Reference = Page " << i << ", In Memory " << ProcessList[newProcessID].pT[i].inMemory;
                            std::cout << " Free Space Available, No Eviction Needed !"<< endl;
                        }
        
                        if(freeMemory.head != nullptr){  /// Allocate the free space to new reference   
                            int filledLocation = AddNewProcess(memoryMap, newProcessID, i, freeMemory, ProcessList);
                            LRU.put(filledLocation);
                            if(i == 0){ /// Check whether process is just started
                                printMemoryMap(memoryMap);
                                swappedInCount++;
                                auto timenow = chrono::system_clock::to_time_t(chrono::system_clock::now());
                                std::cout << "*ENTER : Current timestamp: " << ctime(&timenow) << " Process " << newProcessID << " ENTERING, Size " << ProcessList[newProcessID].size 
                                          << " pages, Service Duration " << ProcessList[newProcessID].serviceDuration << " sec" << endl;
                            }
                        }
                        

                        if (i >= ProcessList[newProcessID].size - 1){ /// Remove the process if last page of the process has been referenced (waiting for 10 ml before removing)
                            removeCompletedProcess(ProcessList, memoryMap, freeMemory, newProcessID);
                            printMemoryMap(memoryMap);
                        }
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto endTime = std::chrono::steady_clock::now(); // Get the end time
        auto durationInSec = std::chrono::duration_cast<std::chrono::seconds>(endTime - start);
        std::cout << "\nPages Referred : " << requestCount << endl;
        if(!reference && durationInSec > std::chrono::seconds(60)) /// Running simulation only for 1 min.
            break;
    }
    return (swappedInCount+25); /// 25 process were pre-filled


}


int main() {

    int choice;
    std::cout << "Choose to Run : \n 1. FIFO 5 Runs \n 2. FIFO 100 Page References \n 3. LRU 5 Runs \n 4. LRU 100 Page References \n"  ;
    std:: cin >> choice;

    auto ExStart = std::chrono::steady_clock::now(); /// Get the start time

    // Variables to track hit/miss ratio
    /// Calculate Average Hit/Miss Ratio for 5 Runs (FIFO)
    double avgHitRatio = 0.0;
    double avgMissRatio = 0.0;
    double avgSwappedInCount = 0.0;

    vector<double>  FIFOtotalMissesRatio(5, 0.000);
    vector<double>  FIFOtotaltotalHitsRatio(5, 0.000);
    vector<int> FIFOSwappedInCount(5,0);
    bool Reference = false;

    switch(choice) 
    {
        case 1: 
        {
            /*********************************************** Simulation 1 : FIFO Algorithm 5 runs ***********************************************/
            for(int simulation = 0; simulation < 5; simulation++)
            {
                Reference = false;
                /// Generate 150 process and corresponding pages
                map<int, Process> ProcessList;
                generateProcesses(ProcessList);

                /// Generate memory map
                vector<MemoryPage> memoryMap(PAGES_IN_MEMORY, {false, -1, -1}); /// Initialize it to Occupied = false, Process id = -1, pageNumber = -1

                /// Generate random processes and add them to the job queue
                JobQueue jobQueue;

                /// Pre-filling the Memory by 4 pages of each process and putting others in jobQueue
                fillMemory(memoryMap, ProcessList, jobQueue);

                FreeMemory freeMemory;

                int totalHits = 0;
                int totalMisses = 0;
                int swappedInCount = simulateFIFO(ProcessList, memoryMap, jobQueue, freeMemory, totalHits, totalMisses, Reference);

                /// Calculate hit/miss ratio
                int totalReferences = totalHits + totalMisses;
                double hitRatio = static_cast<double>(totalHits) / totalReferences;
                double missRatio = static_cast<double>(totalMisses) / totalReferences;
                
                FIFOtotaltotalHitsRatio[simulation]=static_cast<double>(hitRatio);
                FIFOtotalMissesRatio[simulation] = static_cast<double>(missRatio);
                FIFOSwappedInCount[simulation] = swappedInCount;
                std::cout << "Hit Ratio : " << hitRatio << endl;
                std::cout << "Miss Ratio : " << missRatio << endl;
                std::cout << "Swap In : " << swappedInCount << endl;
            }

            /// Calculate Average Hit/Miss Ratio for 5 Runs (FIFO)
            avgHitRatio = static_cast<double>(accumulate(FIFOtotaltotalHitsRatio.begin(), FIFOtotaltotalHitsRatio.end(), 0.0)) / TOTAL_RUNS;
            avgMissRatio = static_cast<double>(accumulate(FIFOtotalMissesRatio.begin(), FIFOtotalMissesRatio.end(), 0.0)) / TOTAL_RUNS;
            avgSwappedInCount = accumulate(FIFOSwappedInCount.begin(), FIFOSwappedInCount.end(), 0) / TOTAL_RUNS;
            
            /// Display hit/miss ratio
            std::cout << "FIFO Average Hit Ratio: " << avgHitRatio << endl;
            std::cout << "FIFO Average Miss Ratio: " << avgMissRatio << endl;
            std::cout << "FIFO Average Swapped In Process Count: " << avgSwappedInCount << endl;
            break;
        }

        case 2:
        {
            /*********************************************** Simulation 2 : FIFO Algorithm 100 referance ***********************************************/
            Reference = true;
            /// Generate 150 process and corresponding pages
            map<int, Process> ProcessList;
            generateProcesses(ProcessList);

            /// Generate memory map
            vector<MemoryPage> memoryMap(PAGES_IN_MEMORY, {false, -1, -1}); /// Initialize it to Occupied = false, Process id = -1, pageNumber = -1

            /// Generate random processes and add them to the job queue
            JobQueue jobQueue;

            /// Pre-filling the Memory by 4 pages of each process and putting others in jobQueue
            fillMemory(memoryMap, ProcessList, jobQueue);

            FreeMemory freeMemory;

            int totalHits = 0;
            int totalMisses = 0;
            int swappedInCount = simulateFIFO(ProcessList, memoryMap, jobQueue, freeMemory, totalHits, totalMisses, Reference);

            /// Calculate hit/miss ratio
            int totalReferences = totalHits + totalMisses;
            double hitRatio = static_cast<double>(totalHits) / totalReferences;
            double missRatio = static_cast<double>(totalMisses) / totalReferences;
            std::cout << "Hit Ratio : " << hitRatio << endl;
            std::cout << "Miss Ratio : " << missRatio << endl;
            std::cout << "Swap In : " << swappedInCount << endl;
            break;
        }
        case 3:
        {
             /*********************************************** Simulation 3 : LRU Algorithm 5 runs ***********************************************/
            for(int simulation = 0; simulation < 5; simulation++)
            {
                Reference = false;
                /// Generate 150 process and corresponding pages
                map<int, Process> ProcessList;
                generateProcesses(ProcessList);

                /// Generate memory map
                vector<MemoryPage> memoryMap(PAGES_IN_MEMORY, {false, -1, -1}); /// Initialize it to Occupied = false, Process id = -1, pageNumber = -1

                /// Generate random processes and add them to the job queue
                JobQueue jobQueue;

                /// Pre-filling the Memory by 4 pages of each process and putting others in jobQueue
                fillMemory(memoryMap, ProcessList, jobQueue);

                FreeMemory freeMemory;

                int totalHits = 0;
                int totalMisses = 0;
                int swappedInCount = simulateLRU(ProcessList, memoryMap, jobQueue, freeMemory, totalHits, totalMisses, Reference);

                /// Calculate hit/miss ratio
                int totalReferences = totalHits + totalMisses;
                double hitRatio = static_cast<double>(totalHits) / totalReferences;
                double missRatio = static_cast<double>(totalMisses) / totalReferences;
                
                FIFOtotaltotalHitsRatio[simulation]=static_cast<double>(hitRatio);
                FIFOtotalMissesRatio[simulation] = static_cast<double>(missRatio);
                FIFOSwappedInCount[simulation] = swappedInCount;
                std::cout << "Hit Ratio : " << hitRatio << endl;
                std::cout << "Miss Ratio : " << missRatio << endl;
                std::cout << "Swap In : " << swappedInCount << endl;
            }

            /// Calculate Average Hit/Miss Ratio for 5 Runs (FIFO)
            avgHitRatio = static_cast<double>(accumulate(FIFOtotaltotalHitsRatio.begin(), FIFOtotaltotalHitsRatio.end(), 0.0)) / TOTAL_RUNS;
            avgMissRatio = static_cast<double>(accumulate(FIFOtotalMissesRatio.begin(), FIFOtotalMissesRatio.end(), 0.0)) / TOTAL_RUNS;
            avgSwappedInCount = accumulate(FIFOSwappedInCount.begin(), FIFOSwappedInCount.end(), 0) / TOTAL_RUNS;
            
            /// Display hit/miss ratio
            std::cout << "LRU Average Hit Ratio: " << avgHitRatio << endl;
            std::cout << "LRU Average Miss Ratio: " << avgMissRatio << endl;
            std::cout << "LRU Average Swapped In Process Count: " << avgSwappedInCount << endl;
            break;
        }

        case 4:
        {
            /*********************************************** Simulation 4 : LRU Algorithm 100 referance ***********************************************/
            Reference = true;
            /// Generate 150 process and corresponding pages
            map<int, Process> ProcessList;
            generateProcesses(ProcessList);

            /// Generate memory map
            vector<MemoryPage> memoryMap(PAGES_IN_MEMORY, {false, -1, -1}); /// Initialize it to Occupied = false, Process id = -1, pageNumber = -1

            /// Generate random processes and add them to the job queue
            JobQueue jobQueue;

            /// Pre-filling the Memory by 4 pages of each process and putting others in jobQueue
            fillMemory(memoryMap, ProcessList, jobQueue);

            FreeMemory freeMemory;

            int totalHits = 0;
            int totalMisses = 0;
            int swappedInCount = simulateLRU(ProcessList, memoryMap, jobQueue, freeMemory, totalHits, totalMisses, Reference);

            /// Calculate hit/miss ratio
            int totalReferences = totalHits + totalMisses;
            double hitRatio = static_cast<double>(totalHits) / totalReferences;
            double missRatio = static_cast<double>(totalMisses) / totalReferences;

            std::cout << "Hit Ratio : " << hitRatio << endl;
            std::cout << "Miss Ratio : " << missRatio << endl;
            std::cout << "Swap In : " << swappedInCount << endl;
            break;
        }
        default:
            std::cout << "Invalid choice." << std::endl;
            break;

    }

    auto ExEndTime = std::chrono::steady_clock::now(); // Get the end time
    auto durationInSec = std::chrono::duration_cast<std::chrono::seconds>(ExEndTime - ExStart);
    std::cout << "\nProgram Execution Time = " << durationInSec.count() << " sec." << endl;
    return 0;
}