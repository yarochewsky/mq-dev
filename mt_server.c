#include "zhelpers.h"
#include <pthread.h>
#include <unistd.h>


static void* worker_routine(void* ctx)
{
  void* receiver = zmq_socket(ctx, ZMQ_REP);
  zmq_connect(receiver, "inproc://workers");

  while (1) {
    char* string = s_recv(receiver);
    printf("Received request [%s]\n", string);
    free(string);
    sleep(1);
    s_send(receiver, "world");
  }
  zmq_close(receiver);
  return NULL;
}

int main(void)
{
  void* ctx = zmq_ctx_new();
  
  void* clients = zmq_socket(ctx, ZMQ_ROUTER);
  zmq_bind(clients, "tcp://*:5555");

  void* workers = zmq_socket(ctx, ZMQ_DEALER);
  zmq_bind(workers, "inproc://workers");

  for (int i = 0; i < 5; i++) {
    pthread_t worker;
    pthread_create(&worker, NULL, worker_routine, ctx);
  }

  zmq_proxy(clients, workers, NULL);

  zmq_close(clients);
  zmq_close(workers);
  zmq_ctx_destroy(ctx);

  return 0;
}
