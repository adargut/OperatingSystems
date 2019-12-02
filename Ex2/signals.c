#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
//----------------------------------------------------
void my_signal_handler( int        signum,
                        siginfo_t* info,
                        void*      ptr)
{
    printf("Signal sent from process %lu\n",
           (unsigned long) info->si_pid);
}
//----------------------------------------------------
int main()
{
    // Structure to pass to the registration syscall
    struct sigaction new_action;
    memset(&new_action, 0, sizeof(new_action));
    // Assign pointer to our handler function
    new_action.sa_sigaction = my_signal_handler;
    // Setup the flags
    new_action.sa_flags = SA_SIGINFO | SA_RESTART;
    // Register the handler
    if( 0 != sigaction(SIGUSR1, &new_action, NULL) )
    {
        printf("Signal handle registration " "failed. %s\n",
               strerror(errno));
        return -1;
    }
    while( 1 )
    {
        sleep( 1 );
        printf("Meditating\n");
    }
    return 0;
}
//================== END OF FILE =====================
