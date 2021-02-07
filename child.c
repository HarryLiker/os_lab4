#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <threads.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#include "shrmem.h"

int string_validation(char *line) {
    int i = 0;
    while (line[i] != '\0') {
        i++;
    }
    if (line[i-1] == '.' || line[i-1] == ';') {
        return 0;
    }
    return -1;
}

int main(int argc, char *argv[]) {

    int fd2 = shm_open(BackingFile, O_RDWR | O_CREAT, AccessPerms);

    int fd3 = shm_open(BackingFile2, O_RDWR | O_CREAT, AccessPerms);

    sem_t *sem_pointer = sem_open(SemaphoreName, O_CREAT, AccessPerms, 2);
    if (sem_pointer == SEM_FAILED) {
        perror("Can't initialize semaphore!\n");
        exit(EXIT_FAILURE);
    }

    char *mapping_file = mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);

    char *mapping_file2 = mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, fd3, 0);

    char *string;
    int value;
    while(1) {
        sem_wait(sem_pointer);
        sem_getvalue(sem_pointer, &value);
        //printf("Значение семафора: %d", value);
        if (mapping_file[0] != '\0') {
            int value;
            int count = sem_getvalue(sem_pointer, &value);
            if (mapping_file[0] == EOF) {
                sem_post(sem_pointer);
                break;
            }
            strcpy(string, mapping_file);
            if (string_validation(string) == 0) {
                printf("%s\n", string);
                memset(mapping_file, '\0', getpagesize());
            } else {
                strcpy(mapping_file2, string);
                memset(mapping_file, '\0', getpagesize());
            }
            sem_getvalue(sem_pointer, &value);
        }
        sem_post(sem_pointer);
    }

    
    munmap(mapping_file, getpagesize());
    close(fd2);
    munmap(mapping_file2, getpagesize());
    close(fd3);
    sem_close(sem_pointer);
    return 0;
    
}