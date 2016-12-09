/******************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************/

#include "helper.h"

void *producer (void *id);
void *consumer (void *id);

//struct to be passed into the thread_create function as arguement
struct thread_args{
  int NO_JOBS;
  int QUEUE_SIZE;
  int* BUFFER;
  int* THREAD_ID_P;
  int* THREAD_ID_C;
  int SEM_ID;
  int* FILL;
  int* USE;
};

int main (int argc, char **argv)
{
  //Fetch command line arguements.
  if(argc != 5){
    cout<<"Invalid number of command line arguement"<<endl;
  }
  int queue_size = check_arg(argv[1]);
  int no_job = check_arg(argv[2]);
  int no_producer = check_arg(argv[3]);
  int no_consumer = check_arg(argv[4]);
  if(queue_size<0||no_job<0||no_producer<0||no_consumer<0){
    cout<<"Invalid arguement."<<endl;
    return 0;
  }

  //Initialise semaphores
  int sem_id =  sem_create(SEM_KEY,3);
  // sem_id[0]: mutex
  if(sem_init(sem_id,0,1)<0){
    sem_close(sem_id);
    return 0;
  }
  //sem_id[1]: number of jobs in the queue
  if(sem_init(sem_id,1,0)<0){
    sem_close(sem_id);
    return 0;
  }
  //sem_id[2]: number of empty space in the queue
  if(sem_init(sem_id,2,queue_size)<0){
    sem_close(sem_id);
    return 0;
  }
  sem_attach(SEM_KEY);

  //Initialise arguements and variables
  srand(time(0));
  int buffer[queue_size];
  int job_in_queue = 0;
  int job_consume = 0;
  int thread_id_p = 0;
  int thread_id_c = 0;
  thread_args arguement;
  arguement.NO_JOBS = no_job;
  arguement.QUEUE_SIZE = queue_size;
  arguement.BUFFER = buffer;
  arguement.SEM_ID = sem_id;
  arguement.FILL = &job_in_queue; // Producers' filling index in buffer
  arguement.USE = &job_consume; // Consumers' consuming index in buffer
  arguement.THREAD_ID_P = &thread_id_p; // pointer to thread id for producers
  arguement.THREAD_ID_C = &thread_id_c; // pointer to thread id for consumers

  pthread_t producerid[no_producer];
  pthread_t consumerid[no_consumer];
  //Create threads for producer
  for(int i=0; i<no_producer;i++){
    pthread_create (&producerid[i], NULL, producer, (void *) &arguement);
  }
  //Create threads for consumer
  for(int i=0; i<no_consumer; i++){
    pthread_create (&consumerid[i], NULL, consumer, (void *) &arguement);
  }
  //Wait for producers finish their jobs
  for(int i=0; i<no_producer; i++){
    pthread_join(producerid[i], NULL);
  }
  //Wait for consumers finish their jobs
  for(int i=0; i<no_consumer; i++){
    pthread_join(consumerid[i], NULL);
  }

  cout<<"Program finished!"<<endl;

  //Clean up semaphores
  sem_close(sem_id);
  return 0;
}

void *producer(void *parameter)
{
  thread_args* args = (thread_args*) parameter;
  //Fetch thread ID for the current thread (critical session).
  sem_wait(args->SEM_ID,0);
  int thread_id = ++*(args->THREAD_ID_P);
  sem_signal(args->SEM_ID,0);

  int sleep_time=0;

  //Create jobs
  for(int i = 0; i < args->NO_JOBS; i++){
    sleep(sleep_time);
    //Set random duration between 1-10 for jobs
    int duration = (rand()%10)+1;
    //semaphore for ensuring any empty space is available in 20 seconds
    int result = sem_waittime(args->SEM_ID,2);
    if(result == -1 && errno == EAGAIN){
      sem_wait(args->SEM_ID,0);
      cout<<"Producer("<<thread_id<<"): Time out."<<endl;
      sem_signal(args->SEM_ID,0);
      break;
    }
    //Enter critical session
    sem_wait(args->SEM_ID,0);
    //accessing shared data (i.e. job ID and buffer)
    int job_id = *(args->FILL)+1;
    args->BUFFER[job_id-1] = duration;
    cout<<"Producer("<<thread_id<<"): Job id "<<job_id <<" duration "<<duration<<endl;
    *(args->FILL) =(job_id)% args->QUEUE_SIZE;
    //Exit critical session
    sem_signal(args->SEM_ID,0);
    //Increase the semaphore value of the number of jobs in buffer
    sem_signal(args->SEM_ID,1);
    //Set random sleep time before producing another job
    sleep_time = (rand()%5)+1;
  }

  //Enter critical session for ouput.
  sem_wait(args->SEM_ID,0);
  cout<<"Producer("<<thread_id<<"): No more jobs to generate."<<endl;
  sem_signal(args->SEM_ID,0);
  pthread_exit(0);
}


void *consumer (void *parameter)
{
  thread_args* args = (thread_args*) parameter;
  //Fetch thread ID for the current thread (critical session).
  sem_wait(args->SEM_ID,0);
  int thread_id = ++ *(args->THREAD_ID_C);
  sem_signal(args->SEM_ID,0);

  while(true){
    int sleep_time;
    //Semaphore for ensuring any job is available in 20 seconds
    int result=sem_waittime(args->SEM_ID,1);
    if(result == -1 && errno == EAGAIN){
      break;
    }
    //Enter critical session
    sem_wait(args->SEM_ID,0);
    //Accessing shared data declared in main function
    int job_id = *(args->USE)+1;
    sleep_time = args->BUFFER[job_id-1];
    cout<<"Consumer("<<thread_id<<"): Job id "<<job_id<<" executing sleep duration "<<sleep_time<<endl;
    *(args->USE) = (job_id)% args->QUEUE_SIZE;
    //Exit critical sessino
    sem_signal(args->SEM_ID,0);
    //Increase the semaphore value of the number of empty space in the buffer
    sem_signal(args->SEM_ID,2);
    sleep(sleep_time);
    //Enter critical session for ouput
    sem_wait(args->SEM_ID,0);
    cout<<"Consumer("<<thread_id<<"): Job id "<<job_id<<" completed"<<endl;
    sem_signal(args->SEM_ID,0);
  }

  //Enter critical session for output
  sem_wait(args->SEM_ID,0);
  cout<<"Consumer("<<thread_id<<"): No more jobs left."<<endl;
  sem_signal(args->SEM_ID,0);
  pthread_exit (0);
}
