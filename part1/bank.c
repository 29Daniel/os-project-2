#include <semaphore.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

// Function to sleep for a random time between 0 and 5 seconds
void random_sleep() {
    int sleep_time = rand() % 6;  // Random number between 0 and 5
    sleep(sleep_time);
}

int main(int argc, char **argv)
{
  int fd, i,nloop=10,zero=0,*counter_ptr;
  sem_t *mutex;

  //open a file and map it into memory this is to hold the shared counter

  fd = open("bankaccount.txt",O_RDWR|O_CREAT,S_IRWXU);
  write(fd,&zero,sizeof(int));
  counter_ptr = mmap(NULL,sizeof(int),PROT_READ |PROT_WRITE,MAP_SHARED,fd,0);
  close(fd);

  /* create, initialize semaphore */
 if ((mutex = sem_open("semaphore", O_CREAT, 0644, 1)) == SEM_FAILED) {
    perror("semaphore initilization");
    exit(1);
  }
 
  /* parent process - Old Dad*/
  if (fork() == 0) {
  for (;;) {
    random_sleep();
    sem_wait(mutex);
    printf("Dear Old Dad: Attempting to Check Balance\n");
    int random_number = rand();
    int localBalance = *counter_ptr;
    
    if (random_number % 2 == 0) {
      if (localBalance < 100) {
        int deposit_amount = rand() % 101;
        if (deposit_amount % 2 == 0) {
          localBalance += deposit_amount;
          printf("Dear Old Dad: Deposits $%d / Balance = $%d\n", deposit_amount, localBalance);
          *counter_ptr = localBalance;
        } else {
          printf("Dear old Dad: Doesn't have any money to give\n");
        }
      } else {
        printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", localBalance);
      }
    } else {
      printf("Dear Old Dad: Last Checking Balance = $%d\n", localBalance);
    }
    sem_post(mutex);
  }
    exit(0);
  }

  /* child process - Poor Student*/
  for (;;) {
      random_sleep();
      sem_wait(mutex);
      printf("Poor Student: Attempting to Check Balance\n");
      int random_number = rand();
      int localBalance = *counter_ptr;
      if (random_number % 2 == 0) {
        int need = rand() % 51;
        printf("Poor Student needs $%d\n", need);
        
        if (need <= localBalance) {
          localBalance -= need;
          printf("Poor Student: Withdraws $%d / Balance = $%d\n", need, localBalance);
          *counter_ptr = localBalance;
        } else {
          printf("Poor Student: Not Enough Cash ($%d)\n", localBalance);
        }
      } else {
        printf("Poor Student: Last Checking Balance = $%d\n", localBalance);
      }
      sem_post(mutex);
    }
  exit(0);
}
