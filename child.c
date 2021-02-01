#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#define MAP_SIZE 4096
char empty = 1;
char *empty_string = &empty;

char *file1_name = "file1_mapped";
//char *file2_name = "file2_mapped";

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
    // Открытие файла для маппинга
    int fd = open(file1_name, O_RDWR | O_CREAT, S_IWRITE | S_IREAD);
    char *file = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (file == MAP_FAILED) {
        perror("Can't map a file!\n");
        exit(2);
    }
    while(strcmp(file, empty_string) == 0) {}
    if (file[0] == EOF) {
        if (munmap(file, MAP_SIZE) < 0) {
            perror("Can't unmap file!\n");
            exit(0);
        }
    }
    /*
    for (int i = 0; i < sizeof(file); i++) {
        char c = file[i];
        printf("%c", c);
    }
    char *string = (char *)malloc(strlen(file) * sizeof(char));
    strcpy(string, file);
    printf("%s\n", string);
    fflush(stdout);
    stpcpy(file, empty_string);
    free(string);
    */
}