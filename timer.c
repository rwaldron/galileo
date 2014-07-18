/* from: 
 http://man7.org/linux/man-pages/man2/timer_create.2.html */
 
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>

#define CLOCKID CLOCK_REALTIME
#define SIG SIGRTMIN

#define errExit(msg) do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

static void print_siginfo(siginfo_t *si)
{
    timer_t *tidp;
    int overrun;

    tidp = si->si_value.sival_ptr;

    printf("    sival_ptr = %p; ", si->si_value.sival_ptr);
    printf("    *sival_ptr = 0x%lx\n", (long) *tidp);

    overrun = timer_getoverrun(*tidp);
    if (overrun == -1)
         errExit("timer_getoverrun");
    else
         printf("    overrun count = %d\n", overrun);
}

static void handler(int sig, siginfo_t *si, void *uc)
{
/* Note: calling printf() from a signal handler is not
strictly correct, since printf() is not async-signal-safe;
see signal(7) */

    printf("Caught signal %d\n", sig);
    print_siginfo(si);
 //   signal(sig, SIG_IGN);
}

int timer(int sleep_s, int f_nanos)
{
    timer_t timerid;
    struct sigevent sev;
    struct itimerspec its;
    long long freq_nanosecs;
    sigset_t mask;
    struct sigaction sa;

    /* Establish handler for timer signal */

    printf("Establishing handler for signal %d\n", SIG);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIG, &sa, NULL) == -1)
    errExit("sigaction");

    /* Block timer signal temporarily */
    //a signal mask is a set of blcocked signals
   /* printf("Blocking signal %d\n", SIG);
    sigemptyset(&mask);
    sigaddset(&mask, SIG);
    if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1)
        errExit("sigprocmask");
*/
    /* Create the timer */

     sev.sigev_notify = SIGEV_SIGNAL;
     sev.sigev_signo = SIG;
     sev.sigev_value.sival_ptr = &timerid;
     if (timer_create(CLOCKID, &sev, &timerid) == -1)
        errExit("timer_create");

     printf("timer ID is 0x%lx\n", (long) timerid);

     /* Start the timer */

     freq_nanosecs = f_nanos;
     its.it_value.tv_sec = freq_nanosecs / 1000000000;
     its.it_value.tv_nsec = freq_nanosecs % 1000000000;
     its.it_interval.tv_sec = its.it_value.tv_sec;
     its.it_interval.tv_nsec = its.it_value.tv_nsec;

     if (timer_settime(timerid, 0, &its, NULL) == -1)
        errExit("timer_settime");

     /* Sleep for a while; meanwhile, the timer may expire
              multiple times */
     printf("Sleeping for %d seconds\n", sleep_s);
     sleep(sleep_s);
    printf("woken up\n");
     /* Unlock the timer signal, so that timer notification
              can be delivered */

 //    printf("Unblocking signal %d\n", SIG);
 //    if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1)
  //      errExit("sigprocmask");

     exit(EXIT_SUCCESS);
}
