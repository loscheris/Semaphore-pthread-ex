/******************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************/

#include "helper.h"

void *producer (void *id);
void *consumer (void *id);

int main (int argc, char **argv)
{
  int queue_size = check_arg(argv[1]);
  int no_job = check_arg(argv[2]);
  int no_producer = check_arg(argv[3]);
  int no_consumer = check_arg(argv[4]);
  if(queue_size<0||no_job<0||no_producer<0||no_consumer<0){
    return 0;
  }



  pthread_t producerid;
  int parameter = 5;

  pthread_create (&producerid, NULL, producer, (void *) &parameter);

  pthread_join (producerid, NULL);

  return 0;
}

void *producer(void *parameter)
{

  // TODO

  int *param = (int *) parameter;

  cout << "Parameter = " << *param << endl;

  pthread_exit(0);
}

void *consumer (void *id)
{
    // TODO

  pthread_exit (0);

}
