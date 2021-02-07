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

// Функция чтения строки
char *getstring() {
    int length = 0;
    int capasity = 10;
    char *str = (char *)malloc(10 * sizeof(char));
    if (str == NULL) {
        perror("Can't read a string!\n");
        exit(6);
    }
    char c;
    while ((c = getchar()) != '\n') {
        str[length++] = c;
        if (c == EOF) {
            break;
        }
        if (length == capasity) {
            capasity *= 2;
            str = (char *)realloc(str, capasity * sizeof(char));
            if (str == NULL) {
                perror("Can't read a string!\n");
                exit(6);
            }
        }
    }
    str[length] = '\0';
    return str;
}


int main() {
    char filename[256];
    printf("Введите имя файла: ");
    scanf("%s", filename);



    int fd2 = shm_open(BackingFile, O_RDWR | O_CREAT, AccessPerms);

    int fd3 = shm_open(BackingFile2, O_RDWR | O_CREAT, AccessPerms);

    sem_t *sem_pointer = sem_open(SemaphoreName, O_CREAT, AccessPerms, 2);
    if (sem_pointer == SEM_FAILED) {
        perror("Can't initialize semaphore!\n");
        exit(EXIT_FAILURE);
    }
    ftruncate(fd2, getpagesize());

    ftruncate(fd3, getpagesize());
    char *mapping_file = mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);
    if (mapping_file == MAP_FAILED) {
        perror("Can't map a file!\n");
        exit(EXIT_FAILURE);
    }

    
    char *mapping_file2 = mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, fd3, 0);
    if (mapping_file2 == MAP_FAILED) {
        perror("Can't map a file!\n");
        exit(EXIT_FAILURE);
    }
    
    memset(mapping_file, '\0', getpagesize());
    memset(mapping_file2, '\0', getpagesize());
    FILE *file_to_write = fopen(filename, "w");
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork error!\n");
        exit(EXIT_FAILURE);
    }
    
    if (pid > 0) {
        int value;
        int count = sem_getvalue(sem_pointer, &value);
        if (value++ < 2) {
            sem_post(sem_pointer);
        }

        while(1) {
            sem_getvalue(sem_pointer, &value);
            if (value == 2) {
                if (mapping_file2[0] != '\0') {
                    sem_wait(sem_pointer);
                    int count = sem_getvalue(sem_pointer, &value);
                    char *string = (char*) malloc(strlen(mapping_file2) * sizeof(char));
                    strcpy(string, mapping_file2);
                    printf("Error string: %s\n", string);
                    memset(mapping_file2, '\0', getpagesize());
                    free(string);
                    sem_post(sem_pointer);
                    continue;
                }
                sem_wait(sem_pointer);
                char *str = getstring();
                if (str[0] != EOF) {
                    strcpy(mapping_file, str);
                    sem_post(sem_pointer);    
                } else {
                    mapping_file[0] = EOF;
                    sem_post(sem_pointer);
                    break;
                }
            } else {
                if (mapping_file2[0] != '\0') {
                    sem_wait(sem_pointer);
                    int count = sem_getvalue(sem_pointer, &value);
                    char *string = (char*) malloc(strlen(mapping_file2) * sizeof(char));
                    strcpy(string, mapping_file2);
                    printf("Error string %s\n", string);
                    memset(mapping_file2, '\0', getpagesize());
                    free(string);
                    sem_post(sem_pointer);
                    continue;
                }
            }
            fflush(stdout);
        }
    }

    if (pid == 0) {
        printf("Дочерний процесс\n");
        dup2(fileno(file_to_write), STDOUT_FILENO);
        execl("./child.out", "child", NULL);
    }

    munmap(mapping_file, getpagesize());
    close(fd2);
    munmap(mapping_file2, getpagesize());
    close(fd3);
    sem_close(sem_pointer);
    shm_unlink(BackingFile);
    shm_unlink(BackingFile2);
    fclose(file_to_write);
    return 0;
}