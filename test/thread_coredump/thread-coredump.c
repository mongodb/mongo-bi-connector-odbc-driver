#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

pthread_mutex_t the_mutex;
pthread_cond_t the_cond;

struct timeval start;
int do_abort = 0;

void thr_msg(char* msg, ...)
{
  va_list args;
  struct timeval now;
  gettimeofday(&now, 0);
  va_start(args, msg);
  fprintf(stderr, "%d ms: pid %d, thread id = %d : ",
	  (now.tv_sec - start.tv_sec) * 1000 +
	  (now.tv_usec - start.tv_usec)/1000,
	  getpid(), pthread_self());
  vfprintf(stderr, msg, args);
  fprintf(stderr, "\n");
  fflush(stderr);
  va_end(args);
}

void handle_coredump(int sig)
{
  thr_msg("Got signal %d, exiting", sig);
  exit(1);
}

void* run(void* arg)
{
  pthread_detach(pthread_self());
  thr_msg("starting...");
  if(arg)
  {
    if(do_abort)
    {
      thr_msg("will abort now, good bye");
      abort();      
    }
    else
    {
      char* p = 0;
      thr_msg("will coredump now, good bye");
      *p = 1;
    }
  }
  thr_msg("main going to sleep");
  pthread_mutex_lock(&the_mutex);
  thr_msg("main got mutex, waiting on cond");
  pthread_cond_wait(&the_cond, &the_mutex);
  pthread_mutex_unlock(&the_mutex);
  thr_msg("main woke up, good bye");
  return 0;
}

int main(int c, char** argv)
{
  int i, n = 5;
  gettimeofday(&start, 0);
  signal(SIGSEGV, handle_coredump);
  pthread_mutex_init(&the_mutex, 0);
  pthread_cond_init(&the_cond, 0);
  if(c > 1 && *(argv[1]) == 'a')
    do_abort = 1;
  
  for(i = 0; i < n; i++)
  {
    pthread_t thd;
    if(pthread_create(&thd, 0, run, (void*)((i == n - 1) ? 1 : 0 )))
    {
      fprintf(stderr, "Could not create thread, errno = %d\n", errno);
      exit(1);
    }
  }
  thr_msg("main going to sleep");
  usleep(100000);
  thr_msg("main woke up");
  pthread_mutex_lock(&the_mutex);
  pthread_cond_broadcast(&the_cond);
  pthread_mutex_unlock(&the_mutex);
  thr_msg("main sent broadcast, going to sleep");
  sleep(2);
  thr_msg("main woke up again");
  return 0;
}
