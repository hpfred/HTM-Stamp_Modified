#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include "timer.h"


#define DEFAULT_DURATION                10000
#define DEFAULT_INITIAL                 256
#define DEFAULT_NB_THREADS              1
#define DEFAULT_RANGE                   0xFFFF
#define DEFAULT_SEED                    0
#define DEFAULT_UPDATE                  20

#define XSTR(s)                         STR(s)
#define STR(s)                          #s

/* ################################################################### *
 * GLOBALS
 * ################################################################### */

extern "C" {
#include "tm.h"
}

#include "thread.h"

__thread void* rot_readset[1024];
__thread char crot_readset[8192];
__thread int irot_readset[2048];
__thread int16_t i2rot_readset[4096];

unsigned int htm_rot_enabled = 1;
unsigned int allow_rots_ros = 1;
unsigned int allow_htms = 1;


__thread unsigned int local_exec_mode = 0;

__thread unsigned int local_thread_id;

__thread long rs_mask_2 = 0xffffffffffff0000;
__thread long rs_mask_4 = 0xffffffff00000000;
__thread long offset = 0;
__thread char* p;
__thread int* ip;
__thread int16_t* i2p;
__thread long moffset = 0;
__thread long moffset_2 = 0;
__thread long moffset_6 = 0;

__thread unsigned long rs_counter = 0;

unsigned int allow_stms = 0;

unsigned int ucb_levers = 4;
unsigned long ucb_trials[4];
unsigned long total_trials;
unsigned int selected_lever[50000];

long *hashmap;

static volatile int stop;

static int N_BUCKETS = 512;
static int L_BUCKET = 10;


TM_CALLABLE
long hm_update_htm(TM_ARGDECL long index, long val)
{
        long j;
	long found = 0;
	long item;
	for(j = index; j <index+L_BUCKET; j++){
		item = TM_SHARED_READ(hashmap[j]);
		if(item == val)
			found = j;
	}
	if(found)
		TM_SHARED_WRITE(hashmap[found],val);
}


TM_CALLABLE
long hm_lookup_htm(TM_ARGDECL long index, long val)
{
	long found = 0;
	long j;
	long item;
	for(j = index; j <index+L_BUCKET; j++){
		item = TM_SHARED_READ(hashmap[j]);
                if(item == val)
                        found = j;
        } 
	return found;
}

TM_CALLABLE
long hm_update_stm(TM_ARGDECL long index, long val)
{
        long j;
        long found = 0; 
	long item;
        for(j = index; j <index+L_BUCKET; j++){
		long item  = TM_SHARED_READ(hashmap[j]);
                if(item == val)
                        found = j;
        } 
        if(found)
                TM_SHARED_WRITE(hashmap[found],val);
}

TM_CALLABLE
long hm_lookup_stm(TM_ARGDECL long index, long val)
{
	long found;
        long j;
	long item;
        for(j = index; j <index+L_BUCKET; j++){
		item = TM_SHARED_READ(hashmap[j]);
                if(item == val)
                        found = j;
        }
	return found;
}


TM_CALLABLE
long priv_update_htm(TM_ARGDECL long val)
{
    return hm_update_htm(TM_ARG (val % N_BUCKETS), val);
}


TM_CALLABLE
long priv_lookup_htm(TM_ARGDECL long val)
{
    return hm_lookup_htm(TM_ARG (val % N_BUCKETS), val);
}


TM_CALLABLE
long priv_update_stm(TM_ARGDECL long val)
{
    return hm_update_stm(TM_ARG (val % N_BUCKETS), val);
}

TM_CALLABLE
long priv_lookup_stm(TM_ARGDECL long val)
{
    return hm_lookup_stm(TM_ARG (val % N_BUCKETS), val);
}



long set_update(TM_ARGDECL long val)
{
    int res = 0;
    int ro = 0;
    TM_BEGIN_ID(0);
    res = (local_exec_mode == 3 || local_exec_mode == 1 || local_exec_mode == 4) ? priv_update_stm(TM_ARG val) : priv_update_htm(TM_ARG val);
    TM_END_ID(0);

    return res;
}


long set_contains(TM_ARGDECL long  val)
{
    long res = 0;

    int ro = 1;
    TM_BEGIN_ID(2);
    res = (local_exec_mode == 3 || local_exec_mode == 1 || local_exec_mode == 4) ? priv_lookup_stm(TM_ARG val) : priv_lookup_htm(TM_ARG val);
    TM_END_ID(2);

    return res;
}

#include <sched.h>
  long range;
  int update;
  unsigned long nb_add;
  unsigned long nb_remove;
  unsigned long nb_contains;
  unsigned long nb_found;
  unsigned long nb_aborts;
  unsigned int nb_threads;
  unsigned int seed;
  long operations;

void test(void *data)
{

  TM_THREAD_ENTER();

  unsigned int mySeed = getenv("PREFETCHING") ? seed + sched_getcpu()/8 : seed + sched_getcpu();
  long myOps = getenv("PREFETCHING") ? operations / (nb_threads/2) : operations / nb_threads;

  long val = -1;
  int op;

  while (myOps > 0) {
    op = rand_r(&mySeed) % 100;
    if (op < update) {
        val = (rand_r(&mySeed) % range) + 1;
        set_update(TM_ARG  val);
    } else {
      long tmp = (rand_r(&mySeed) % range) + 1;
      set_contains(TM_ARG tmp);
    }

    myOps--;
  }

  TM_THREAD_EXIT();
}

# define no_argument        0
# define required_argument  1
# define optional_argument  2

MAIN(argc, argv) {
    TIMER_T start;
    TIMER_T stop;


  struct option long_options[] = {
    // These options don't set a flag
    {"help",                      no_argument,       NULL, 'h'},
    {"duration",                  required_argument, NULL, 'd'},
    {"initial-size",              required_argument, NULL, 'i'},
    {"num-threads",               required_argument, NULL, 'n'},
    {"range",                     required_argument, NULL, 'r'},
    {"seed",                      required_argument, NULL, 's'},
    {"buckets",                   required_argument, NULL, 'b'},
    {"update-rate",               required_argument, NULL, 'u'},
    {NULL, 0, NULL, 0}
  };

  int i, c;
  long val;
  operations = DEFAULT_DURATION;
  unsigned int initial = DEFAULT_INITIAL;
  nb_threads = DEFAULT_NB_THREADS;
  range = DEFAULT_RANGE;
  update = DEFAULT_UPDATE;
  N_BUCKETS = 512;

  while(1) {
    i = 0;
    c = getopt_long(argc, argv, "hd:i:n:b:r:s:u:", long_options, &i);

    if(c == -1)
      break;

    if(c == 0 && long_options[i].flag == 0)
      c = long_options[i].val;

    switch(c) {
     case 0:
       /* Flag is automatically set */
       break;
     case 'h':
       printf("intset -- STM stress test "
              "(hash map)\n"
              "\n"
              "Usage:\n"
              "  intset [options...]\n"
              "\n"
              "Options:\n"
              "  -h, --help\n"
              "        Print this message\n"
              "  -d, --duration <int>\n"
              "        Test duration in milliseconds (0=infinite, default=" XSTR(DEFAULT_DURATION) ")\n"
              "  -i, --initial-size <int>\n"
              "        Number of elements to insert before test (default=" XSTR(DEFAULT_INITIAL) ")\n"
              "  -n, --num-threads <int>\n"
              "        Number of threads (default=" XSTR(DEFAULT_NB_THREADS) ")\n"
              "  -r, --range <int>\n"
              "        Range of integer values inserted in set (default=" XSTR(DEFAULT_RANGE) ")\n"
              "  -s, --seed <int>\n"
              "        RNG seed (0=time-based, default=" XSTR(DEFAULT_SEED) ")\n"
              "  -u, --update-rate <int>\n"
              "        Percentage of update transactions (default=" XSTR(DEFAULT_UPDATE) ")\n"
         );
       exit(0);
     case 'd':
       operations = atoi(optarg);
       break;
     case 'b':
       N_BUCKETS = atoi(optarg);
       break;
     case 'i':
       initial = atoi(optarg);
       break;
     case 'n':
       nb_threads = atoi(optarg);
       break;
     case 'r':
       range = atoi(optarg);
       break;
     case 's':
       seed = atoi(optarg);
       break;
     case 'u':
       update = atoi(optarg);
       break;
     case '?':
       printf("Use -h or --help for help\n");
       exit(0);
     default:
       exit(1);
    }
  }

  nb_threads = getenv("PREFETCHING") ? nb_threads*2 : nb_threads;
  L_BUCKET = initial / N_BUCKETS;

  if (seed == 0)
    srand((int)time(0));
  else
    srand(seed);

  SIM_GET_NUM_CPU(nb_threads);
  TM_STARTUP(nb_threads);
  P_MEMORY_STARTUP(nb_threads);
  thread_startup(nb_threads);

  hashmap = (long *) malloc(initial*sizeof(long));


  /* Populate set */
  printf("Adding %d entries to set\n", initial);
  for (i = 0; i < initial; i++) {
	hashmap[i] = i;
  }
  puts("Added\n");
  seed = rand();
  TIMER_READ(start);
  GOTO_SIM();
//startEnergyIntel();

 thread_start(test, NULL);

  GOTO_REAL();
  TIMER_READ(stop);

//double energy = endEnergyIntel();
  puts("done.");
  printf("\nTime = %0.6lf\n", TIMER_DIFF_SECONDS(start, stop));
//printf("Energy = %0.6lf\n", energy);
  fflush(stdout);

  TM_SHUTDOWN();
  P_MEMORY_SHUTDOWN();
  GOTO_SIM();
  thread_shutdown();
  MAIN_RETURN(0);
}
