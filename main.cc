/******************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************/

#include "helper.h"

void *producer (void *id);
void *consumer (void *id);

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
  int queue_size = check_arg(argv[1]);
  int no_job = check_arg(argv[2]);
  int no_producer = check_arg(argv[3]);
  int no_consumer = check_arg(argv[4]);
  if(queue_size<0||no_job<0||no_producer<0||no_consumer<0){
    return 0;
  }

  srand(time(0));
  int buffer[queue_size];
  int job_in_queue = 0;
  int job_consume = 0;
  //  int* fill = &job_in_queue;
  //  int* use = &job_consume; 
  int thread_id_p = 0;
  int thread_id_c = 0;
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
  
  pthread_t producerid[no_producer];
  pthread_t consumerid[no_consumer];
  

  struct thread_args arguement;
  arguement.NO_JOBS = no_job;
  arguement.QUEUE_SIZE = queue_size;
  arguement.BUFFER = buffer;
  arguement.SEM_ID = sem_id;
  arguement.FILL = &job_in_queue;
  arguement.USE = &job_consume;
  arguement.THREAD_ID_P = &thread_id_p;
  arguement.THREAD_ID_C = &thread_id_c;

  
  for(int i=0; i<no_producer;i++){
    pthread_create (&producerid[i], NULL, producer, (void *) &arguement);  
  }

  
  for(int i=0; i<no_consumer; i++){
    pthread_create (&consumerid[i], NULL, consumer, (void *) &arguement);
  }

  for(int i=0; i<no_producer; i++){
    pthread_join(producerid[i], NULL);
  }

  for(int i=0; i<no_consumer; i++){
    pthread_join(consumerid[i], NULL);
  }

  cout<<"Finished"<<endl;
  
  sem_close(sem_id);

  if(errno == EAGAIN){
    cout<<"Time's up"<<endl;
  }


  return 0;
}

void *producer(void *parameter)
{

  // TODO
  struct thread_args* args = (struct thread_args*) parameter;
  
  int thread_id = ++*(args->THREAD_ID_P);
  sem_signal(args->SEM_ID,0);
  //create jobs
  int job[args->NO_JOBS];
  int sleep_time = 0;

  for(int i = 0; i < args->NO_JOBS; i++){
    sleep(sleep_time);
    int duration = (rand()%10)+1;
    // cout<<temp<<" ";
    job[i]=duration;
    
    int result= sem_waittime(args->SEM_ID,2);
    if(result == -1 && errno == EAGAIN){
      cout<<"Producer("<<thread_id<<"): Time out."<<endl;
      break;
    }
    sem_wait(args->SEM_ID,0);
    
    int job_id = *(args->FILL);
    args->BUFFER[job_id] = job[i];
    cout<<"Producer("<<thread_id<<"): Job id "<<job_id <<" duration "<<duration<<endl;
    *(args->FILL) = (job_id + 1)% args->QUEUE_SIZE;
    
    sem_signal(args->SEM_ID,0);
    sem_signal(args->SEM_ID,1);
    sleep_time = (rand()%5)+1;
  }

  sem_wait(args->SEM_ID,0);
  cout<<"Producer("<<thread_id<<"): No more jobs to generate."<<endl;
  sem_signal(args->SEM_ID,0);
  pthread_exit(0);
}


void *consumer (void *parameter)
{
  struct thread_args* args = (struct thread_args*) parameter;
  
  sem_wait(args->SEM_ID,0);
  int thread_id = ++ *(args->THREAD_ID_C);
  sem_signal(args->SEM_ID,0);
  
  while(true){
    int sleep_time;

    int result = sem_waittime(args->SEM_ID,1);
    if(result == -1 && errno == EAGAIN){
      break;
     }
    sem_wait(args->SEM_ID,0);

    int job_id = *(args->USE);
    sleep_time = args->BUFFER[job_id];
    cout<<"Consumer("<<thread_id<<"): Job id "<<job_id<<" executing sleep duration "<<sleep_time<<endl;
    *(args->USE) = (job_id + 1)% args->QUEUE_SIZE;

    sem_signal(args->SEM_ID,0);
    sem_signal(args->SEM_ID,2);

    sleep(sleep_time);
    
    sem_wait(args->SEM_ID,0);
    cout<<"Consumer("<<thread_id<<"): Job id "<<job_id<<" completed"<<endl;
    sem_signal(args->SEM_ID,0);
  }

  sem_wait(args->SEM_ID,0);
  cout<<"Consumer("<<thread_id<<"): No more jobs left."<<endl;
  sem_signal(args->SEM_ID,0);
  
  pthread_exit (0);

}
