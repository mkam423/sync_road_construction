// Mason Kam
// p2.cpp
// Purpose: To practice synchronization of threads and maximize concurrency
//  without any conflicts and race conditions. To model a common roadway
//  occurrence, where a lane is closed and a flag person is directing traffic.
// Input: y or n to run program. If y is entered and programming is running,
//  then any character to end the threads and exit the program
// Processes: Creating multiple threads, locks, and a semaphore in order to
//  synchronize the threads.
// Output: Cars being passed through the road block. Welcome and exit message.

#include<iostream>
#include<cctype>
#include<stdlib.h>
#include<pthread.h>
#include<time.h>
#include<queue>
#include<semaphore.h>
#include<fstream>
using namespace std;


void *produceN(void *arg);
// Thread function to control North side cars

void *produceS(void *arg);
// Thread function to control South side cars

void *consumer(void *arg);
// Thread function to allow cars passing through

int pthread_sleep(int seconds);
// Function to let thread sleep

void northCar();
// Let a car from the north queue through blockade

void southCar();
// Let a car from the south queue through blockade

void setupLogs();
// Sets up output files


const int NEXT_CAR_CHANCE = 8;
const int MAX_ROLL = 10;
const unsigned FULL_LINE = 10;
const int CLEAR = 50;
const char YES = 'y';
const char NO = 'n';
const string CAR_FILE = "car.log";
const string FLAG_FILE = "flagperson.log";


pthread_mutex_t mutex_var;
pthread_mutex_t north_mutex;
pthread_mutex_t south_mutex;
sem_t sem;
queue <string> N;
queue <string> S;
int num;
int done = false;
ofstream carLog;
ofstream flagLog;

int main()
{
  pthread_t north, south, flag;
  char run;
  bool play = true;

  
  for(int i = 0; i < CLEAR; i++)
	cout << endl;

  cout << "Run program? (y/n): ";
  cin >> run;
  while(tolower(run) != YES && tolower(run) != NO){
	cout << "Must enter y or n: ";
	cin >> run;
  }
  
  while(tolower(run) == YES){
	cout << "Enter any character to exit." << endl;
	cout << "S represents South and N represents North" << endl;
	cout << "Program will run shortly..." << endl;
	
	pthread_sleep(3); // Time to read briefing

	setupLogs();
	
	cout << "Creating threads..." << endl;
	pthread_sleep(2); // Time to read briefing
	
	if(-1 == pthread_create(&flag, NULL, &consumer, NULL)){
	  perror("pthread_create3");
	  return -1;
	}	
    if(-1 == pthread_create(&north, NULL, &produceN, NULL)){
	  perror("pthread_create1");
	  return -1;
    }
    if(-1 == pthread_create(&south, NULL, &produceS, NULL)){
	  perror("pthread_create2");
	  return -1;
    }
  
    while(play){
	  cin >> run; // Any character terminate
	  play = false;
    }
	
    done = true;
	run = NO;

	cout << "Waiting for threads to finish..." << endl;

	pthread_join(north, NULL);
    pthread_join(south, NULL);
	sem_post(&sem);
    pthread_join(flag, NULL);

	carLog.close();
	flagLog.close();
	sem_destroy(&sem);
  }


  cout << "Exiting program." << endl << endl << endl;
  
  return 0;
}


void setupLogs()
{
  carLog.open(CAR_FILE);
  flagLog.open(FLAG_FILE);

  carLog << "Contents will be printed in order of the following: " << endl;
  carLog << "CarID, Direction, Arrival Time, Start Time, and End Time.";
  carLog << endl << endl;

  flagLog << "Contents will be printed in the format State and Time";
  flagLog << endl << endl;

  srand(time(0));
  sem_init(&sem, 0, 0);
}


void *produceN(void *arg)
{
  int roll = 0;
  time_t rawTime;
  
  while(!done){
	pthread_mutex_lock (&mutex_var);
	pthread_mutex_lock(&north_mutex);
	
	if(S.size() == 0 && N.size() == 0)
	  sem_post(&sem);
		
	do {
	  time(&rawTime);
	  N.push(ctime(&rawTime));
   	  roll = rand() % MAX_ROLL;
   	} while(roll < NEXT_CAR_CHANCE);

	cout << "Number of cars from North: " << N.size() << endl;
	
	pthread_mutex_unlock(&north_mutex);
	pthread_mutex_unlock(&mutex_var);

	pthread_sleep(20);
  }
  return 0;
}


void *produceS(void *arg)
{
  int roll = 0;
  time_t rawTime;
  
  while(!done){
	pthread_mutex_lock (&mutex_var);
	pthread_mutex_lock(&south_mutex);

   	if(S.size() == 0 && N.size() == 0)
	  sem_post(&sem);
	
	do {
	  time(&rawTime);
	  S.push(ctime(&rawTime));
	  roll = rand() % MAX_ROLL;
	} while(roll < NEXT_CAR_CHANCE);

	cout << "Number of cars from South: " << S.size() << endl;
 
	pthread_mutex_unlock(&south_mutex);
	pthread_mutex_unlock (&mutex_var);
	
	pthread_sleep(20);
  }
  return 0;
}


void *consumer(void *arg)
{
  time_t flagTime;

  cout << endl << endl;
  if(S.size() == 0 && N.size() == 0){
  cout << "Flag person sleep." << endl;
  time(&flagTime);
  flagLog << "Sleep: " << ctime(&flagTime);
  sem_wait(&sem);
  }
  
  while(!done){
	pthread_mutex_lock(&mutex_var);
	cout << "Flag person awake." << endl;
	pthread_mutex_unlock(&mutex_var);
	time(&flagTime);
	flagLog << " Awaken: " << ctime(&flagTime) << endl;
	
	while(N.size() > 0 || S.size() > 0){
	  //Let one side through at a time
	  if(N.size() < FULL_LINE && S.size() < FULL_LINE){
		while(N.size() != 0){
		  northCar();
		  while(S.size() >= FULL_LINE) // Check if S has 10+ cars
			southCar();
		}
		while(S.size() != 0){
		  southCar();
		  while(N.size() >= FULL_LINE) // Check if N has 10+ cars
		    northCar();
		}
	  //South side 10+ cars
	  } else if (S.size() >= FULL_LINE && N.size() < FULL_LINE){
		while(S.size() != 0){
		  southCar();
   		  while(N.size() >= FULL_LINE) // Check if N has 10+ cars
			northCar();
		}
	  //North side 10+ cars
	  } else if (N.size() >= FULL_LINE && S.size() < FULL_LINE){
		while(N.size() != 0){
   		  northCar();
		  while(S.size() >= FULL_LINE) // Check if N has 10+ cars
			southCar();
		}
	  //Both sides 10+ cars
      } else {
		while(S.size() >= FULL_LINE)
		  southCar();
		while(N.size() >= FULL_LINE)
		  northCar();
	  }
    }

	
	if(!done){
	  cout << "Flag person sleep." << endl << endl << endl;
	  time(&flagTime);
	  flagLog << "Sleep: " << ctime(&flagTime);
	  
	  sem_wait(&sem);
	}
  }
  return 0;
}


void northCar()
{
  string carTime;
  time_t rawTime;
  
  carTime = N.front();
  
  pthread_mutex_lock(&north_mutex);
  N.pop();
  pthread_mutex_unlock(&north_mutex);
  
  num++;
  
  cout << "Car " << num << " from N going through road." << endl;
  carLog << num << endl;
  carLog << "N" << endl;
  carLog << carTime;
  
  time(&rawTime);
  carLog << ctime(&rawTime);
  
  pthread_sleep(1);

  time(&rawTime);
  carLog << ctime(&rawTime) << endl;
}


void southCar()
{
  string carTime;
  time_t rawTime;

  carTime = S.front();

  pthread_mutex_lock(&south_mutex);
  S.pop();
  pthread_mutex_unlock(&south_mutex);

  num++;
  
  cout << "Car " << num << " from S going through road." << endl;
  carLog << num << endl;
  carLog << "S" << endl;
  carLog << carTime;
  
  time(&rawTime);
  carLog << ctime(&rawTime);
  
  pthread_sleep(1);

  time(&rawTime);
  carLog << ctime(&rawTime) << endl;
}


int pthread_sleep(int seconds)
{
  pthread_mutex_t mutex;
  pthread_cond_t conditionVar;
  struct timespec expireTime;

  if(pthread_mutex_init(&mutex, NULL))
	return -1;

  if(pthread_cond_init(&conditionVar, NULL))
	return -1;

  expireTime.tv_sec = (unsigned int)time(NULL) + seconds;
  expireTime.tv_nsec = 0;

  return pthread_cond_timedwait(&conditionVar, &mutex, &expireTime);  
}

