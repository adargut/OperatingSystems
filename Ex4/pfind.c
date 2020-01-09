#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/syscall.h>




static int          NUM_THREADS;
#define          DT_DIR       4
static int              counter    = 0;
pthread_cond_t cond;
static int sleepy_threads=0;
pthread_mutex_t  lock;

//====================================================
typedef struct Node_t {
    DIR* dir;
    char* father_dir;
    struct Node_t* next;
} Node;

Node *createNode( const char *data, const char *father ) {

    char *resolved_path = malloc(PATH_MAX);
    Node *res           = malloc(sizeof(Node));
    res->next           = NULL;
    res->father_dir     = father;
    if (father == "") {
        father = "/";
    }
    else {
        strcat(father, "/");
    }
    printf("father is:%s and son:%s\n", father, data);
    char fn[255];
    strcpy( fn, data );
    strcat( fn, father );
//    strcat( fn, ".txt" );
    printf("res is %s\n", fn);
//    strcat(father, data);
    res->dir = opendir(fn);
    res->father_dir = fn;

    if (NULL == res->dir) {
        printf("ERROR in opening directory: %s\n", fn);
        exit(-1);
    }
    printf("added to queue: %s\n", fn);
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
//    // --- Lock: enter CS -------------------------------------
//    rc = pthread_mutex_lock(&lock);
//    if( 0 != rc )
//    {
//        printf( "ERROR in pthread_mutex_lock(): "
//                "%s\n", strerror( rc ) );
//        exit( -1 );
//    }

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
//    rc = pthread_mutex_unlock(&lock);
//    if( 0 != rc )
//    {
//        printf( "ERROR in pthread_mutex_unlock(): "
//                "%s\n", strerror( rc ) );
//        exit( -1 );
//    }
}

Node *dequeue ( Queue *queue ) {

    int rc;
    // --- Lock: enter CS -------------------------------------
//    rc = pthread_mutex_lock(&lock);
//    if( 0 != rc )
//    {
//        printf( "ERROR in pthread_mutex_unlock(): "
//                "%s\n", strerror( rc ) );
//        exit( -1 );
//    }
//
    if (is_empty(queue)) return NULL;
    queue->size--;
    Node *res   = queue->head;
    queue->head = queue->head->next;

    // --- Unlock: exit CS -------------------------------------
//    rc = pthread_mutex_unlock(&lock);
//    if( 0 != rc )
//    {
//        printf( "ERROR in pthread_mutex_unlock(): "
//                "%s\n", strerror( rc ) );
//        exit( -1 );
//    }

    return res;
}

//====================================================
bool match_query(char *search_query, char *filename)
{
    int query_len = strlen(search_query);
    int file_len  = strlen(filename);
    int streak    = 0;
    int i         = 0;
    int j         = 0;
    int start_pos = 0;

    if (query_len > file_len) return false;

    while (i < file_len) {
        if (search_query[j++] == filename[i++]) {
            streak += 1;
            if (streak == query_len) return true;
        }
        else {
            streak = 0;
            j      = 0;
            i      = start_pos;
            start_pos++;
        }
    }
    return false;
}

typedef struct search_args {
    Queue* queue_arg;
    char*  search_arg;
} Search_args;

Search_args *create_search_args(Queue *queue, char *search_query) {

    Search_args *search_args = malloc(sizeof(search_args));
    search_args->queue_arg   = queue;
    search_args->search_arg  = search_query;

    return search_args;
}

//====================================================

void concurrent_search(Search_args *search_args)
{
    char *search_query        = search_args->search_arg;
    Queue *queue              = search_args->queue_arg;
    Node* head                = NULL;
    DIR* curr_dir;
    struct dirent* curr_entry;

    while(true) {
        while (!is_empty(queue)) {
            printf("hiya\n");
            while (head == NULL) {
                pthread_mutex_lock( &lock );
                head = dequeue(queue);
                printf("got head%s\n", head->father_dir);
            }

            pthread_mutex_unlock( &lock );
            curr_dir = head->dir;

            while ((curr_entry = readdir(curr_dir)) != NULL) {
                // .. or . found
                if (!strcmp(".", curr_entry->d_name) || !strcmp("..", curr_entry->d_name)) {
                    continue;
                }
                // Directory found
                if (DT_DIR == curr_entry->d_type) {
                    printf("what is father dir? %s\n", head==NULL);
                    Node *new_entry = createNode(curr_entry->d_name, head->father_dir);
                    pthread_mutex_lock( &lock );
                    enqueue(queue, new_entry);
                    pthread_cond_signal(&cond);
                    pthread_mutex_unlock( &lock );
                }
                    // File found
                else if (match_query(search_query, curr_entry->d_name)) {
                    printf("%s\n", curr_entry->d_name);
                    pid_t tid = syscall(SYS_gettid);
                    printf("found from %d\n", tid);
                }
            }


        }
        printf("hello\n");

        pthread_mutex_lock( &lock );
        sleepy_threads++;
        if(sleepy_threads == NUM_THREADS){ /*All n threads are sleeping*/
            /*call another thread*/
            pthread_cond_signal(&cond);
            /*exit current thread*/
            pthread_mutex_unlock( &lock );
            pthread_exit(0);
        }
        /*Sleep untill signal*/
        pthread_cond_wait(&cond, &lock);
        sleepy_threads--;
        pthread_testcancel();
        pthread_mutex_unlock( &lock );
    }



}

//====================================================
int main (int argc, char *argv[])
{
    char*          search_query;
    int            rc;
    long           t;
    void*          status;
    Node*          node;

    if (argc != 4)
    {
        printf("ERROR in args count\n");
        exit(-1);
    }

    // --- Initialize queue ----------------------------
//    char cwd[PATH_MAX];
//    if (getcwd(cwd, sizeof(cwd)) != NULL) {
//        printf("Current working dir: %s\n", cwd);
//    } else {
//        perror("getcwd() error");
//        return 1;
//    }
//    printf("what is cwd? %s\n", cwd);
    printf("hi\n");
    node                     = createNode(argv[1], "");
    Queue *queue             = createQueue();
    search_query             = argv[2];
    Search_args *search_args = create_search_args(queue, search_query);
    enqueue(queue, node);


    // --- Initialize threads ----------------------------
    NUM_THREADS = atoi(argv[3]);
    pthread_t thread[NUM_THREADS];

    // --- Initialize mutex ----------------------------
    rc = pthread_mutex_init( &lock, NULL );
    if( rc )
    {
        printf("ERROR in pthread_mutex_init(): "
               "%s\n", strerror(rc));
        exit(-1);
    }
    pthread_cond_init (&cond, NULL);

    // --- Launch threads ------------------------------
    for( t = 0; t < NUM_THREADS; ++t )
    {
        printf("Main: creating thread %ld\n", t);
        rc = pthread_create( &thread[t],
                             NULL,
                             concurrent_search,
                             (void*) search_args);
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
