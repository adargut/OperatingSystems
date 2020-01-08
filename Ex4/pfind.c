#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>


#define          NUM_THREADS  5
static long      TOTAL;
int              counter    = 0;
pthread_mutex_t  lock;

//====================================================
int next_counter(void)
{
    pthread_mutex_lock( &lock );
    int temp = ++counter;
    pthread_mutex_unlock( &lock );
    return temp;

    //return __sync_fetch_and_add(&counter, 1);
}

//====================================================
typedef struct Node_t {

    struct dirent* dirEnt;
    struct Node_t* next;
} Node;

Node *createNode( const char *data ) {

    Node *res  = malloc(sizeof(Node));
    res->next  = NULL;
    res->data  = data;

    return res;
}

typedef struct Queue {

    Node *head;
    Node *tail;
    int  size;
} Queue;

Queue *createQueue() {

    Queue *queue = malloc(sizeof(Queue));
    queue->size  = 0;
    queue->head  = NULL;
    queue->tail  = NULL;

    return queue;
}

bool is_empty(Queue *queue) {
    return queue->size == 0;
}

void enqueue( Queue *queue, Node *element ) {

    int rc;
    // --- Lock: enter CS -------------------------------------
    rc = pthread_mutex_lock(&lock);
    if( 0 != rc )
    {
        printf( "ERROR in pthread_mutex_lock(): "
                "%s\n", strerror( rc ) );
        exit( -1 );
    }

    queue->size++;
    if (queue->head == NULL) {
        queue->head = element;
        queue->tail = element;
    }
    else {
        queue->tail->next = element;
        queue->tail       = element;
    }

    // --- Unlock: exit CS -------------------------------------
    rc = pthread_mutex_unlock(&lock);
    if( 0 != rc )
    {
        printf( "ERROR in pthread_mutex_unlock(): "
                "%s\n", strerror( rc ) );
        exit( -1 );
    }
}

Node *dequeue ( Queue *queue ) {

    int rc;
    // --- Lock: enter CS -------------------------------------
    rc = pthread_mutex_lock(&lock);
    if( 0 != rc )
    {
        printf( "ERROR in pthread_mutex_unlock(): "
                "%s\n", strerror( rc ) );
        exit( -1 );
    }

    if (is_empty(queue)) return NULL;
    queue->size--;
    Node *res   = queue->head;
    queue->head = queue->head->next;

    // --- Unlock: exit CS -------------------------------------
    rc = pthread_mutex_unlock(&lock);
    if( 0 != rc )
    {
        printf( "ERROR in pthread_mutex_unlock(): "
                "%s\n", strerror( rc ) );
        exit( -1 );
    }

    return res;
}

//====================================================
void concurrent_search(char *search_query, Queue *queue)
{
    Node *head = dequeue(queue);
    struct dirent x;

    while (NULL != curr_file = readdir(head->dirEnt)) {
        if (DT_DIR != curr_file->d_type) {
            printf("%s\n", curr_file->name);
        }
    }
}

//====================================================
int main (int argc, char *argv[])
{
    DIR*           root_dir;
    struct dirent* dirEnt;
    char*          search_query;
    int            num_threads;
    int            rc;
    long           t;
    void*          status;

    if (argc != 4)
    {
        printf("ERROR in args count");
        exit(-1);
    }

    // --- Initialize threads ----------------------------
    num_threads = argv[3];
    pthread_t thread[num_threads];

    // --- Open root directory ----------------------------
    root_dir     = opendir(argv[1]);
    search_query = argv[2];

    // --- Initialize mutex ----------------------------
    rc = pthread_mutex_init( &lock, NULL );
    if( rc )
    {
        printf("ERROR in pthread_mutex_init(): "
               "%s\n", strerror(rc));
        exit(-1);
    }

    // --- Launch threads ------------------------------
    for( t = 0; t < NUM_THREADS; ++t )
    {
        printf("Main: creating thread %ld\n", t);
        rc = pthread_create( &thread[t],
                             NULL,
                             concurrent_search,
                             (void*) t);
        if (rc) {
            printf("ERROR in pthread_create():"
                   " %s\n", strerror(rc));
            exit( -1 );
        }
    }

    // --- Wait for threads to finish ------------------
    for( t = 0; t < NUM_THREADS; ++t )
    {
        rc = pthread_join(thread[t], &status);
        if (rc)
        {
            printf("ERROR in pthread_join():"
                   " %s\n", strerror(rc));
            exit( -1 );
        }
        printf("Main: completed join with thread %ld "
               "having a status of %ld\n",t,(long)status);
    }

    // --- Epilogue -------------------------------------
    printf("Main: program completed. Exiting."
           " Counter = %d\n",counter);

    pthread_mutex_destroy(&lock);
    pthread_exit(NULL);
}
//=================== END OF FILE ====================
