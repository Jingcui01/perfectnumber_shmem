/*************************************************************

THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING

A TUTOR OR CODE WRITTEN BY OTHER STUDENTS-JING CUI

*************************************************************/


#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#define KEY (key_t)98997
#define MessKey (key_t)78990
//#define MessKeyR (key_t)78991
#define BIT_SIZE 1024000
#define PERF_NUM 20
#define PROC_NUM 20

int signum;

struct process
{
    pid_t pid;
    pid_t find;
    pid_t test;
    pid_t skip;
};

struct shmem
{
    uint8_t bitmap[BIT_SIZE/8];
    pid_t perfect[20];
    struct process proc[20];
};