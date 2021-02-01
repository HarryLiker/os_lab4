#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>

#define MAP_SIZE 4096

// Пустая строка в качестве сигнала
char empty = 1;
char *empty_string = &empty;

char *file1_name = "file1_mapped";
//char *file2_name = "file2_mapped";

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
    char *filename;
    printf("Введите имя файла: ");
    filename = getstring();
    int file_to_write = open(filename, O_WRONLY | O_CREAT, S_IWRITE | S_IREAD);
    //int fd1[2]; // Файловый дескриптор 1
    //int fd2[2]; // Файловый дескриптор 2
    if (file_to_write < 0) {
        perror("Can't open file!\n");
        exit(1);
    }
    // Создание файла для маппинга
    int fd1 = open(file1_name, O_RDWR | O_CREAT, S_IWRITE | S_IREAD);
    //int fd2 = open(file2_name, O_RDWR | O_CREAT, S_IWRITE | S_IREAD);
    if (fd1 < 0) {
        perror("Can't open file!\n");
        exit(1);
    }
    if (write(fd1, empty_string, sizeof(empty_string)) < 0) {
        perror("Can't write to file!\n");
        exit(1);
    }
    /*
    if (write(fd2, empty_string, sizeof(empty_string)) < 0) {
        perror("Can't write to file!\n");
        exit(1);
    }
    */
    char *file1 = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0);
    //char *file2 = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);
    if (file1 == MAP_FAILED) {
        perror("Can't map a file!\n");
        exit(2);
    }
    pid_t pid = fork(); // Создание дочернего процесса
    if (pid < 0) {
        perror("Can't create child process!\n");
        exit(3);
    } else if (pid > 0) { // Родительский процесс
        while(1) {
            char *s = getstring();
            //printf("COPIYED STRING: %s\n", s);
            strcpy(file1, s);
            if (s[0] == EOF) {
                printf("LAST STRING: %s", s);
                //strcpy(file1, s);
                break;
            }
        }
        if (munmap(file1, MAP_SIZE) < 0) {
            perror("Can't unmap file!\n");
            exit(4);
        }
        if (close(fd1) < 0) {
            perror("Can't close file!\n");
            exit(5);
        }
        if (remove(file1_name) < 0) {
            perror("Can't delete file!\n");
            exit(6);
        }
    } else { // Дочерний процесс
        // Перенаправление стандартного ввода для дочернего процесса

        // Перенаправление стаднартного вывода для дочернего процесса
        if (dup2(file_to_write, STDOUT_FILENO) < 0) {
            perror("Can't redirect stdout for child process!\n");
            exit(5);
        } 
        char *arr[] = {filename, NULL};
        execv("./child.out", arr);
        perror("Can't execute child process!\n");
        exit(8);
    }
}