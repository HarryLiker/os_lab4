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

//char *mapping_file_name = "mapping_file_1";

int main() {
    char filename[256];
    printf("Введите имя файла: ");
    scanf("%s", filename);


    //int fd = open(mapping_file_name, O_RDWR | O_CREAT, S_IWRITE | S_IREAD);

    int fd2 = shm_open(BackingFile, O_RDWR | O_CREAT, AccessPerms);

    int fd3 = shm_open(BackingFile2, O_RDWR | O_CREAT, AccessPerms);

    sem_t *sem_pointer = sem_open(SemaphoreName, O_CREAT, AccessPerms, 2);
    if (sem_pointer == SEM_FAILED) {
        perror("Can't initialize semaphore!\n");
        exit(EXIT_FAILURE);
    }
    ftruncate(fd2, getpagesize());

    ftruncate(fd3, getpagesize());
    //ftruncate(fd, getpagesize());
    /*
    if (fd < 0) {
        perror("Can't open file!\n");
        exit(EXIT_FAILURE);
    }
    */
    //char *mapping_file = mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
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
    

    //pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    memset(mapping_file, '\0', getpagesize());
    memset(mapping_file2, '\0', getpagesize());
    FILE *file_to_write = fopen(filename, "w");
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork error!\n");
        exit(EXIT_FAILURE);
    }
    
    if (pid > 0) {
        printf("Родительский процесс\n");
        //char string[256];
        /*
        while (scanf("%s", string) > 0) {
            sem_wait(sem_pointer);
            //pthread_mutex_lock(&lock);
            printf("Прочитана строка: %s\n", string);
            strcpy(mapping_file, string);
            //pthread_mutex_unlock(&lock);
            sem_post(sem_pointer);
        }
        */

        while(1) {
            /*
            char *str = getstring();
            printf("Один\n");
            sem_wait(sem_pointer);
            if (mapping_file2[0] != '\0') {
                printf("Обнаружено, что в родительский процесс отправлено сообщение\n");
                char *string = (char*) malloc(strlen(mapping_file2) * sizeof(char));
                strcpy(string, mapping_file2);
                printf("Error string %s", string);
                memset(mapping_file2, '\0', getpagesize());
                free(string);
                sem_post(sem_pointer);
                continue;
            }
            printf("Два\n");
            sem_wait(sem_pointer);
            if (str[0] != EOF) {
                //printf("Длинная строка прочитана!%s\n", str);
                strcpy(mapping_file, str);
                sem_post(sem_pointer);
            } else {
                mapping_file[0] = EOF;
                sem_post(sem_pointer);
                break;
            }
            printf("Три\n");
            */

            char *str = getstring();
            if (str[0] != EOF) {
                sem_wait(sem_pointer);
                printf("Длинная строка прочитана!%s\n", str);
                strcpy(mapping_file, str);
                sem_post(sem_pointer);
            } else {
                mapping_file[0] = EOF;
                break;
            }
            /*
            if (mapping_file2[0] != '\0') {
                printf("Обнаружено, что в родительский процесс отправлено сообщение\n");
                char *string = (char*) malloc(strlen(mapping_file2) * sizeof(char));
                strcpy(string, mapping_file2);
                printf("Error string %s", string);
                memset(mapping_file2, '\0', getpagesize());
                free(string);
                sem_post(sem_pointer);
                continue;
            }
            */
            
            //printf("Три\n");











            
            if (mapping_file2[0] != '\0') {
                sem_wait(sem_pointer);
                printf("Обнаружено, что в родительский процесс отправлено сообщение");
                char *string = (char*) malloc(strlen(mapping_file2) * sizeof(char));
                strcpy(string, mapping_file2);
                printf("Error string %s", string);
                memset(mapping_file2, '\0', getpagesize());
                free(string);
                sem_post(sem_pointer);
                continue;
            } else {
                sem_wait(sem_pointer);
                char *str = getstring();
                if (str[0] != EOF) {
                    //sem_wait(sem_pointer);
                    printf("Длинная строка прочитана!%s\n", str);
                    strcpy(mapping_file, str);
                    sem_post(sem_pointer);
                } else {
                    mapping_file[0] = EOF;
                    sem_post(sem_pointer);
                    break;
                }
                //sem_post(sem_pointer);
            }
            
            fflush(stdout);
        }
    }

    if (pid == 0) {
        printf("Дочерний процесс\n");
        //dup2(fileno(file_to_write), STDOUT_FILENO);
        execl("./child.out", "child", NULL);
        //printf("Ha-ha it writes in file!\n");
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