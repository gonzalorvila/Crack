#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <crypt.h>
#include <stdlib.h>
#include <string.h>

char a = 'a';
char z = 'z';
char letter = 'a'; 
int found = 0; 

int keysize;
char* target;
char salt[3];

void getSalt(char* target, char* salt)
{
    for (int i = 0; i < 3; i++)
    {
        if (i == 2)
        {
            salt[i] = '\0';
        }
        else
        {
            salt[i] = target[i];
        }
    }
}

void getPassword(int length, int index, char* password, struct crypt_data* data, pthread_mutex_t* mutex)
{
    if (found == 1)
    {
        return;
    }
    for (char i = a; i <= z; i++)
    {
        if (length != 1)
        {
            password[index] = i;
        }

        if (index < length)
        {
            getPassword(length, index + 1, password, data, mutex);
        }
        else
        {
            char* hash = crypt_r(password, salt, data);
            if (strcmp(target, hash) == 0)
            {
                pthread_mutex_lock(mutex);
                found = 1; 
                pthread_mutex_unlock(mutex);
                printf("Found password: %s\n", password);
                return;
            }
        }
    }
}

void* threads(void* input)
{
    pthread_mutex_t* mutex = (pthread_mutex_t*) input;
    while (1)
    {
        if (found != 0)
        {
            return NULL; 
        }
        if (letter > z)
        {
            pthread_mutex_lock(mutex);
            found = 1;
            printf("No matching password found");
            pthread_mutex_unlock(mutex);
        }
        else
        {
            char password[keysize+1];
            password[0] = letter;

            for (int i = 1; i < keysize; i++)
            {
                struct crypt_data data;
                data.initialized = 0;
                password[i] = '\0';
                getPassword(i, 1, password, &data, mutex);
            }
            pthread_mutex_lock(mutex);
            letter++;
            pthread_mutex_unlock(mutex);
        }
    }
    return NULL;
}

int main (int argc, char* argv[])
{
    int numThreads = atoi(argv[1]);
    keysize = atoi(argv[2]);
    target = argv[3];
    pthread_t thread[numThreads];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t *mutexPtr = &mutex;
    getSalt(target, salt);
    int callNum[numThreads];

    if (keysize < 1)
    {
        printf("keysize must be between 1 and 8");
    }

    if (keysize > 8)
    {
        printf("crypt uses only first 8 characters. Setting keysize to ");
        keysize = 8;
    }
    if (numThreads < 1)
    {
        printf("threads  should be greater than 1. Changing to 1 thread.");
        numThreads = 1;
    }


    for (int i = 0; i < numThreads; i++)
    {
         callNum[i] = pthread_create(&thread[i], NULL, threads, mutexPtr);
         if (callNum[i] != 0)
         {
             printf("Error creating a thread %d: %d\n", i+1, callNum[i]);
             exit(-1);
         }
    }
    for (int i = 0; i < numThreads; i++)
    {
        pthread_join(thread[i], NULL);
    }
     return 0;
}

