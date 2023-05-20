// include all the librairies
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

// include the header file
#include "sematube.h"

/* elements which handle the debug mode */

/**
 * @brief If true, print debug message. Else, no debug message.
 *
 */
char debug;

/* elements which handle the shared memory */

/**
 * @brief Semaphore shmem.
 *
 */
void *shmem_sem;
/**
 * @brief Queue shmem.
 *
 */
void *shmem_queue;

/**
 * @brief Create a shared memory area (shmem).
 * Find on StackOverflow.
 *
 * @param size The size of the shmem area une bytes.
 * @return The shmem object.
 */
void *create_shmem(size_t size)
{
    // Our memory buffer will be readable and writable :
    int protection = PROT_READ | PROT_WRITE;

    // The buffer will be shared (meaning other processes can access it), but anonymous (meaning third-party processes cannot obtain an address for it), sol only this process and its children will be able to use it :
    int visibility = MAP_SHARED | MAP_ANONYMOUS;

    // The remaining parameters to 'mmap()' are not important for this use case, but the manpage for 'mmap' explains their purpose.
    return mmap(NULL, size, protection, visibility, -1, 0);
}

/**
 * @brief The write_shmem() function copies sizeof(value) bytes from memory area value to memory area shmem. The memory areas must not overlap.
 *
 * @param shmem The destination memory area.
 * @param value The value to copy.
 * @return A pointer to shmem.
 */
void *write_shmem(void *shmem, void *value)
{
    return memcpy(shmem, value, sizeof(value));
}

/**
 * @brief Useless function. Just to simplify the code.
 *
 * @param shmem The shmem area.
 * @return The shmem area.
 */
void *read_shmem(void *shmem)
{
    return shmem;
}

/* elements which handle the named pipe */

/**
 * @brief The named pipe's path.
 *
 */
const char *named_pipe = "/tmp/sematube_named_pipe";

/**
 * @brief Read the named pipe.
 *
 * @param value A pointer to where you want to get the read value.
 * @return See man read(2).
 */
ssize_t read_pipe(void *value)
{
    int fd = open(named_pipe, O_RDONLY);             // get the file descriptor
    ssize_t retval = read(fd, value, sizeof(value)); // read the pipe
    close(fd);                                       // close the fd
    return retval;
}

/**
 * @brief Write in the names pipe.
 *
 * @param value The value to write.
 * @return See man write(2).
 */
ssize_t write_pipe(void *value)
{
    int fd = open(named_pipe, O_WRONLY);
    ssize_t retval = write(fd, value, sizeof(value));
    close(fd);
    return retval;
}

/* elements which handle the interruptions */

/**
 * @brief Disable all the signal handler. It's use for the atomics operations.
 *
 */
void disable_interrupt()
{
    int i;
    for (i = 1; i <= 31; i++) // there are 31 utile signals
        signal(i, SIG_IGN);   // ignore them
}

/**
 * @brief Set the default handler for all utile signals.
 *
 */
void enable_interrupt()
{
    int i;
    for (i = 1; i <= 31; i++) // there are 31 utile signals
        signal(i, SIG_DFL);   // set the default signal's handler
}

/* elements which handle the fifo queue */

/**
 * @brief Pointer on element of FIFO pile.
 * From Mrs. Russo's class.
 *
 */
typedef struct el *pel;

/**
 * @brief Element of FIFO pile.
 * From Mrs. Russo's class.
 *
 */
typedef struct el
{
    pid_t pid;
    pel next;
} tel;

/**
 * @brief Create a pel object.
 * From Mrs. Russo's class.
 *
 * @param pid Element of the pel structure.
 * @return pel object.
 */
pel createElem(pid_t pid)
{
    pel tmp;
    tmp = (pel)malloc(sizeof(tel));
    tmp->pid = pid;
    tmp->next = NULL;
    return tmp;
}

/**
 * @brief Add an entry to the end of the FIFO pile.
 * From Mrs. Russo's class.
 *
 * @param f The FIFO pile.
 * @param pid The data to store.
 * @return The FIFO pile.
 */
pel add(pel f, pid_t pid)
{
    pel aux = createElem(pid);
    pel tmp = f;
    if (f == NULL)
        return aux;
    else
    {
        while (tmp->next != NULL)
            tmp = tmp->next;
        tmp->next = aux;
        return f;
    }
}

/**
 * @brief Delete the first element of the FIFO pile.
 * From Mrs. Russo's class.
 *
 * @param f The FIFO pile.
 * @return The FIFO pile.
 */
pel erase(pel f)
{
    pel tmp = f;
    if (f != NULL)
    {
        f = f->next;
        free(tmp);
    }
    return f;
}

/* elements which handle the semphore */

/**
 * @brief P() decrements (locks) the semaphore pointed to by S. If the semaphore's value is greater than zero, then the decrement proceeds, and  the function returns, immediately. If the semaphore currently has the value zero, then the call blocks until either it becomes possible to perform the decrement (i.e., the semaphore value rises above zero), or a signal handler interrupts the call.
 * See V() and e().
 *
 * @param S The semaphore.
 * @return EXIT_SUCCESS on success; EXIT_FAILURE on failure.
 */
int P(sematube *sem)
{
    disable_interrupt(); // disableling the interruptions for atomics operations
    if (debug)
        printf("(debug#%i) P function.\n", getpid());
    sem = (sematube *)read_shmem(shmem_sem); // get shmem_sem's sem value
    if (sem->tokens > 0)                     // if there is a token left
    {
        if (debug)
            printf("(debug#%i) tokens decrement.\n", getpid());
        sem->tokens--;
    }
    else // if there isn't any token left
    {
        if (debug)
            printf("(debug#%i) adding process to queue.\n", getpid());
        pel fifo = (pel)read_shmem(shmem_queue); // recover the FIFO pile in her shmem area
        add(fifo, getpid());                     // add a new entry
        write_shmem(shmem_queue, fifo);          // save it in the shmem area
        pid_t pid;
        if (debug)
            printf("(debug#%i) waiting for token.\n", getpid());
        do
        {
            if (debug)
                printf("(debug#%i) reading pipe.\n", getpid());
            read_pipe(&pid); // read the pipe until this process is called to continue
        } while (pid != getpid());
        sem->tokens--;
    }
    write_shmem(shmem_sem, sem); // push the sem in shmem_sem
    enable_interrupt();          // enable the signal's handlers
    return EXIT_SUCCESS;
}

/**
 * @brief V() increments (unlocks) the semaphore pointed to by S. If the semaphore's value consequently becomes greater than zero, then  another process or thread blocked in a P() call will be woken up and proceed to lock the semaphore.
 * See P() and e().
 *
 * @param S The semaphore.
 * @return EXIT_SUCCESS on success; EXIT_FAILURE on failure.
 */
int V(sematube *sem)
{
    disable_interrupt(); // disableling the interruptions for atomics operations
    if (debug)
        printf("(debug#%i) V function.\n", getpid());
    sem = (sematube *)read_shmem(shmem_sem); // get shmem_sem's sem value
    if (sem->tokens < sem->size)             // if it's possible to add another token
    {
        if (debug)
            printf("(debug#%i) tokens increment.\n", getpid());
        sem->tokens++;
        pel fifo = *(pel *)read_shmem(shmem_queue); // read the shmem area to get the FIFO pile
        if (debug)
            printf("(debug#%i) value of fifo : %p.\n", getpid(), fifo);
        if (fifo != NULL)                        // if there is someone in the queue
        {
            pid_t pid = erase(fifo)->pid; // delete it
            if (debug)
                printf("(debug#%i) there's someone in the queue (pid:%i).\n", getpid(), pid);
            write_shmem(shmem_queue, fifo); // update in shmem
            if (debug)
                printf("(debug#%i) writing in pipe.\n", getpid());
            write_pipe(&pid); // broadcast in the pipe the process called pid's
        }
        write_shmem(shmem_sem, sem); // push the sem in shmem_sem
    }
    else
    {
        if (debug)
            printf("(debug#%i) there's already the maximum amount of tokens.\n", getpid());
        enable_interrupt(); // enable the signal's handlers
        return EXIT_FAILURE;
    }
    enable_interrupt(); // enable the signal's handlers
    return EXIT_SUCCESS;
}

/**
 * @brief e() initializes the unnamed semaphore at the address pointed to by S. The n argument specifies the initial value for the semaphore.
 * See V() and P().
 *
 * @param S The semaphore.
 * @param n The initial value for the semaphore.
 * @param debug The boolean for debug mode.
 * @return EXIT_SUCCESS on success; EXIT_FAILURE on failure.
 */
int e(sematube *sem, int n, char bool)
{
    disable_interrupt();             // disableling the interruptions for atomics operations
    debug = bool;                    // set debug value
    pel fifo = NULL;                 // create FIFO pile
    sem->tokens = n;                 // set semaphore structure value's
    sem->size = n;                   // set semaphore structure value's
    shmem_sem = create_shmem(128);   // create shmem for semaphore
    write_shmem(shmem_sem, sem);     // push the sem in shmem_sem
    shmem_queue = create_shmem(128); // create shmem for queue
    write_shmem(shmem_queue, &fifo); // push the queue in shmem_queue
    mkfifo(named_pipe, 0666);        // create the names pipe
    if (debug)
        printf("(debug#%i) Sematube initialized.\n", getpid());
    enable_interrupt(); // enable the signal's handlers
    return EXIT_SUCCESS;
}
