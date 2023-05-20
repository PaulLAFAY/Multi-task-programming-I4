#ifndef SEMATUBE
#define SEMATUBE

/**
 * @brief The sematube structure
 *
 */
typedef struct
{
    int tokens;
    int size;
} sematube;

/**
 * @brief P() decrements (locks) the semaphore pointed to by S. If the semaphore's value is greater than zero, then the decrement proceeds, and  the function returns, immediately. If the semaphore currently has the value zero, then the call blocks until either it becomes possible to perform the decrement (i.e., the semaphore value rises above zero), or a signal handler interrupts the call.
 * See V() and e().
 *
 * @param S The semaphore.
 * @return EXIT_SUCCESS on success; EXIT_FAILURE on failure.
 */
int P(sematube *S);

/**
 * @brief V() increments (unlocks) the semaphore pointed to by S. If the semaphore's value consequently becomes greater than zero, then  another process or thread blocked in a P() call will be woken up and proceed to lock the semaphore.
 * See P() and e().
 *
 * @param S The semaphore.
 * @return EXIT_SUCCESS on success; EXIT_FAILURE on failure.
 */
int V(sematube *S);

/**
 * @brief e() initializes the unnamed semaphore at the address pointed to by S. The n argument specifies the initial value for the semaphore.
 * See V() and P().
 *
 * @param S The semaphore.
 * @param n The initial value for the semaphore.
 * @param debug The boolean for debug mode.
 * @return EXIT_SUCCESS on success; EXIT_FAILURE on failure.
 */
int e(sematube *S, int n, char debug);

#endif