/*
Title : Disk Scheduling Algorithm 
Desciption : Simulating following disk scheduling algorithms: 
                1. First Come-First Serve (FCFS), 
                2. Shortest Seek First (SSF) and 
                3. Elevator (SCAN) algorithms.
Author: Kartiki Dindorkar
*/

#include<iostream>
using namespace std;

#define NUM_OF_REQUESTS 10
#define START_POSITION 50
#define DISK_SIZE 200

/// Generate Random Requests
vector<int> generateRequests(){
    vector<int> requests;
    srand(time(NULL));
    for (int i = 0; i < NUM_OF_REQUESTS; i++){
        int request = rand() % 100;
        requests.push_back(request);
    }
    return requests;
}

/// @brief  Calculating the distance from start position to the next requests
/// @param requests : vector of requests
/// @param startPos : Starting Position
/// @return : Returning Total Distance Travelled
int readingDiskRequests(vector<int> requests, int startPos, int totalDistance){
    //int totalDistance = 0;
	int distance, currentRequest;

    for (int i = 0; i < requests.size(); i++) {
		currentRequest = requests[i];
		distance = abs(currentRequest - startPos);
		totalDistance += distance;
		startPos = currentRequest;
        cout << i+1 << " Reading Track " << currentRequest << ", Distance Moved: " << distance << ", Total Distance Travelled: " << totalDistance <<  endl;
	}
    return totalDistance;
}

void simulateFCFS(vector<int> requests){
    int startPos = START_POSITION;
    int distanceTravelled = readingDiskRequests(requests, startPos, 0);
	cout << "\nFCFS Total Distance : " << distanceTravelled << endl;
}

void simulateSSF(vector<int> requests){
    int startPos = START_POSITION;
    vector<int> SSFRequests = requests;

    /// Sorting the request to get shortest seek first
    sort(SSFRequests.begin(), SSFRequests.end()); 
    int distanceTravelled = readingDiskRequests(SSFRequests, startPos, 0);

    cout << "\nSSF Total Distance : " << distanceTravelled << endl;
}

void simulateSCAN(vector<int> requests){
    int startPos = START_POSITION;
    vector<int> left, right; 

    left.push_back(0);
    //right.push_back(DISK_SIZE-1);

    /// Store requests less than and greater than current head position
    for(int request = 0; request < requests.size(); request++){
        if(requests[request] < START_POSITION) left.push_back(requests[request]);
        else if(requests[request] > START_POSITION) right.push_back(requests[request]);
    }

    sort(right.begin(), right.end()); /// Sorting in Ascending Order
    sort(left.begin(),left.end(), greater<int>()); /// Sorting in Descending Order
   
    /// Went to the left most request from start position (till 0)
    int distanceTravelled = readingDiskRequests(left, startPos, 0);
    
    /// From 0 go to the right most request
    startPos = 0; 
    distanceTravelled = readingDiskRequests(right, startPos, distanceTravelled);

    cout << "\nSCAN (ELEVATOR) Total Distance : " << distanceTravelled << "\n" << endl;  
}

void printVector(vector<int> request){
    cout << "size : " << request.size() << endl;
    cout << "{ ";
    for(auto i : request) 
        cout << i << ", ";
    cout << "}\n";
}

int main()
{
	vector<int> requests = generateRequests();
	
    cout << "\nRequests are : ";
    printVector(requests);
    /// Simulate FCFS Algorithm
    cout << "\n****************** Starting First Come First Serve Algorithm ******************\n" << endl;
	simulateFCFS(requests);

    /// Simulate SSF Algorithm
    cout << "\n****************** Starting Shortest Seek First Algorithm ******************\n" << endl;
    simulateSSF(requests);

    /// Simulate SCAN (ELEVATOR) Algorithm
     cout << "\n****************** Starting SCAN (ELEVATOR) Algorithm ******************\n" << endl;
    simulateSCAN(requests);

	return 0;
}
