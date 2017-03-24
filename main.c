#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdio.h>
#include <pthread.h> 
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <ctype.h>

int sizeOfQueue;
int productsInQueue = 0;
int maxNumOfProducts;
int productsCreated = 0;
int productsConsumed = 0;
int mySeed;
int front = 0;
int rear = -1;
int theQuantum = 0;
pthread_mutex_t the_mutex;
pthread_cond_t condc, condp;

// Global Time metrics
double maxTurn = 0;
double minTurn = 1000;
double turnSum = 0;
double turnAvg = 0;
double maxWait = 0;
double minWait = 1000;
double waitSum = 0;
double waitAvg = 0;
double prodThrough;
double consThrough;

struct product { /* Used to store product info in queue */
	int  product_id;
	double time_stamp; // Creation time
	int life;
	double stop_time; // last De-scheduled time
	double wait_time; // running total of wait time.
};
//declare array of products
struct product *product_queue;
//sets the global variable sizeOfQueue = the input 
void setQueueSize(int size)
{
	int i;
	product_queue = malloc(sizeof(struct product)*size);
	sizeOfQueue = size;
}
//sets global variable equal to input
void setMaxNumOfProds(int products)
{
	maxNumOfProducts = products;
}
//looks at the front of the queue
struct product peek(){
	return product_queue[front];
}
//Checks if the queue is empty, 0 if false, 1 if true
int isEmpty(){
	return productsInQueue == 0;
}
//Checks if queue is full, returns 0 if false, 1 if true
int isFull() {
	return productsInQueue == sizeOfQueue;
}
//Returns the size of the queue
int size() {
	return productsInQueue;
}
//Inserts a product into the queue if the queue is not full
void insert(struct product myProduct)
{
	if(!isFull()) {
		if(rear == sizeOfQueue-1){
			rear = -1;
		}
		product_queue[++rear] = myProduct;
		productsInQueue++;
	}
}
//Removes a product from the queue and returns the removed product
struct product removeProd() {
	struct product prodToBeRemoved = product_queue[front++];
	if(front == sizeOfQueue) {
		front = 0;
	}
	productsInQueue--;
	int test = prodToBeRemoved.product_id;
	return prodToBeRemoved;
} 
//Gets the current system time in milliseconds
double getTimeStamp(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	double time_in_mill = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;

	return time_in_mill;
}
//The classic fibonacci function
void fn(int num) {
	int fib1 = 0, fib2 = 1, fib3, count = 2;
	while (count < num) {
		fib3 = fib1 + fib2;
		count++;
		fib1 = fib2;
		fib2 = fib3;
	}
}
//Checks if the given character array is a number, returns 0 if false, 1 if true
int isNumber(char number[])
{
    int i = 0;

    //checking for negative numbers
    if (number[0] == '-')
    {
        return 0;
    }
    for (; number[i] != 0; i++)
    {
        //if (number[i] > '9' || number[i] < '0')
        if (!isdigit(number[i]))
            return 0;
    }
    return 1;
}

void main(int argc, char** argv) {
	//Check that all input parameters are present
	if (argc == 8) {
	
	//Store all inputs:
	//P1: Number of producer threads
	int k;
	for(k=1;k<8;k++)
	{
		if(!isNumber(argv[k]) && argv[k] != NULL)
		{
			printf("Argument %d is negative, or not a number\n", k);
			return;
		}
	}
	int nProducers = atoi(argv[1]);
	
	//printf("%c", *argv[0]);
	//printf("%i", nProducers);
	//P2: Number of consumer threads
	int nConsumers = atoi(argv[2]);
	//P3: Total number of products to be generated
	int totalProducts = atoi(argv[3]);
	//P4: Size of queue
	int qSize = atoi(argv[4]);
	//P5: 0 = First Come First Serve, 1 = Round Robin
	int scheduling = atoi(argv[5]);
	//P6: Value of Quantum for RR scheduling
	int quantum = atoi(argv[6]);
	if (scheduling == 1) {
		theQuantum = quantum;
	} else {
		theQuantum = 1025;
	}
	//P7: Seed for random number generator
	int seed = atoi(argv[7]);
	mySeed = seed;
	int i;
	int producerIds[nProducers];
	int consumerIds[nConsumers];
	pthread_t producer_thread[nProducers];
	pthread_t consumer_thread[nConsumers];
	void *producer();
	void *consumer();
	setQueueSize(qSize);
	setMaxNumOfProds(totalProducts);
	double startTime = 0;
	double endTime = 0;
	double prodEndTime = 0;
	double runTime = 0;
	double prodRunTime = 0;
	
	pthread_mutex_init(&the_mutex, NULL);	
  	pthread_cond_init(&condc, NULL);		/* Initialize consumer condition variable */
  	pthread_cond_init(&condp, NULL);		/* Initialize producer condition variable */
	srandom(mySeed);
	startTime = getTimeStamp();
	printf("start: %f\n", startTime);
	/* Creating and joining threads*/
	for (i=0;i<nProducers;i++){
		producerIds[i] = i;
		pthread_create(&producer_thread[i],NULL,producer,&producerIds[i]);
	}
	for (i=0;i<nConsumers;i++){
		consumerIds[i] = i;
		pthread_create(&consumer_thread[i],NULL,consumer,&consumerIds[i]);
	}

	for(i=0; i<nProducers;i++) {
		pthread_join(producer_thread[i], NULL);
	}
	prodEndTime = getTimeStamp();
	for(i=0; i<nConsumers;i++) {
		pthread_join(consumer_thread[i],NULL);
	}
	endTime = getTimeStamp();
	runTime = endTime-startTime;
	prodRunTime = prodEndTime-startTime;
	
	printf("\n============\nTime Metrics\n============\n");

	printf("Total Runtime: %f\n", runTime);
	
	// Turnaround Times
	turnAvg = turnSum/maxNumOfProducts;
	printf("minTurn: %f, maxTurn: %f, avgTurn: %f\n", minTurn, maxTurn, turnAvg);
	
	// Wait Times
	waitAvg = waitSum/maxNumOfProducts;
	printf("minWait: %f, maxWait: %f, avgWait: %f\n", minWait, maxWait, waitAvg);
	
	// Throughput
	prodThrough = maxNumOfProducts/(prodRunTime/60000);
	printf("prodThrough: %f\n", prodThrough);

	
	consThrough = maxNumOfProducts/(runTime/60000);
	printf("consThrough: %f\n", consThrough);
	

	} else {
		printf("Use format: ./assign1 [Number of Producers] [Number of Consumers] [Number of Products] [Queue Size] [0 = 1st Come 1st Serve, 1 = Round Robin] [Quantum Time] [Rand Seed]");
		return;
	}

	free(product_queue);
	return;
	
}
/*
*Producer function, parameter is the producer ID
*Exits after the maximum number of products has been created
*/
void *producer(int *q) {
	printf("I am producer thread %d\n", *q);
	while(productsCreated < maxNumOfProducts)
	{
		//locks the mutex in order to access the queue
		pthread_mutex_lock(&the_mutex);
		//if the queue cannot be accessed because it is full, the thread waits.
		while(productsCreated < maxNumOfProducts && productsInQueue >= sizeOfQueue)
		{
			pthread_cond_wait(&condp, &the_mutex);
		}
		//Additional sanity check to ensure that more products than the max cannot be created
		//if more products can be created, the producer thread creates the product and adds
		//it to the queue
		if(productsCreated < maxNumOfProducts)
		{
			//addToQueue
			struct product newProduct;

			newProduct.product_id = productsCreated;

			newProduct.time_stamp = getTimeStamp();

			newProduct.stop_time = newProduct.time_stamp;

			newProduct.life = random()%1024;
			newProduct.wait_time = 0;
			
			insert(newProduct);
			productsCreated++;
			printf("Producer %d has created Product %d\n", *q, newProduct.product_id);
		}
		//signal consumer threads that there are products that can be consumed
		pthread_cond_broadcast(&condc);
		//unlock the mutex to allow other threads to access the queue
		pthread_mutex_unlock(&the_mutex);
		//sleep to ensure that other threads can attempt to access the queue when 
		//the queue is available
		usleep(100000);
	}
	pthread_exit(0);
} 
/*
*Consumer function, parameter is the consumer ID
*Exits after the maximum number of consumed has been consumed
*/
void *consumer(int *q) {
	printf("I am consumer thread %d\n", *q);
	struct product takenProduct;
	int i = 0;
	double takeTime = 0;
	double putTime = 0;
	double turnAround = 0;
	while (productsConsumed < maxNumOfProducts)
	{
		pthread_mutex_lock(&the_mutex);
		while (productsCreated < maxNumOfProducts && productsInQueue < sizeOfQueue)
		{
			pthread_cond_wait(&condc, &the_mutex);
		}
		//take from queue
		if(!isEmpty())
		{
			// Get time 
			takeTime = getTimeStamp();
			takenProduct = removeProd();


		}
		//consume
		// Check if life > quantum
		if(productsConsumed < maxNumOfProducts)
		{
			//RR Case
			if (takenProduct.life > theQuantum) 
			{
				// if true, life = life-quantum and call fn(10) q times 
				takenProduct.life = takenProduct.life - theQuantum;
				for (i = theQuantum; i  > 0; i--) {
					fn(10);
				} 
				// return product to back of queue

				putTime = getTimeStamp(); // Update wait_time and time_stamp
				takenProduct.wait_time += takeTime - takenProduct.stop_time;
				takenProduct.stop_time = putTime;
				insert(takenProduct);
			}			
			else //FCFS Case 
			{
				// else, remove product from queue, call fn(10) l times
				for (i = takenProduct.life; i > 0; i--) {
					fn(10);
				}
				// increment productsConsumed
				productsConsumed++;
				//calculate the new wait times and turnAround times
				putTime = getTimeStamp();
				takenProduct.wait_time += takeTime - takenProduct.stop_time; // just update wait time
				turnAround = putTime - takenProduct.time_stamp;
				//update min and max wait variables if needed
				if (takenProduct.wait_time < minWait){
					minWait = takenProduct.wait_time;
				}
				if (takenProduct.wait_time > maxWait) {
					maxWait = takenProduct.wait_time;
				}
				waitSum += takenProduct.wait_time;
				if (turnAround < minTurn){
					minTurn = turnAround;
				}
				if (turnAround > maxTurn){
					maxTurn = turnAround;
				}
				turnSum += turnAround;
				printf("Consumer %d has consumed Product %d.  Wait Time: %f milliseconds, Turnaround Time: %f\n", *q,takenProduct.product_id, takenProduct.wait_time, turnAround );

			}
		}
		// -- print product's ID
		pthread_cond_broadcast(&condp);
		pthread_mutex_unlock(&the_mutex);
		usleep(100000);
	}
	pthread_exit(0);
}