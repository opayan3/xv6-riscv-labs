#include "kernel/types.h"
#include "kernel/pstat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  if (argc < 2){
    fprintf(2, "Usage:...\n");
    exit(1);
  }

  int initial_time = uptime();

  int pid = fork();
  if (pid < 0){
    fprintf(2, "fork failed\n");
    exit(1);
  }

  if (pid == 0){
    exec(argv[1], &argv[1]);
    fprintf(2, "exec %s failed\n", argv[1]);
    exit(1);
  }

  int status;
  struct rusage ru;
  wait2(&status, &ru);

  int final_time = uptime();
  int elapsed_time = final_time - initial_time;
  int cpu_percent = (elapsed_time>0) ? (ru.cputime * 100) / elapsed_time : 0;

  printf("elapsed time: %d ticks, cpu time: %d ticks, %d%% CPU\n", elapsed_time, ru.cputime, cpu_percent);

  exit(0);
}
