#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>

int main() {
    printf("enter file name:\n> ");
    fflush(stdout);
    char filename[256];
    scanf("%s", filename);

    char pipeName[] = "pipe";
    char mutexName[] = "mutex";
    char mutex2Name[] = "mutex2";
    char pipe1SizeName[] = "size";


    int pipe1 = shm_open(pipeName, O_RDWR | O_CREAT, S_IRWXU);
    int pipe1Size = shm_open(pipe1SizeName, O_RDWR | O_CREAT, S_IRWXU);
    int mutex1 = shm_open(mutexName, O_RDWR | O_CREAT, S_IRWXU);
    int mutex2 = shm_open(mutex2Name, O_RDWR | O_CREAT, S_IRWXU);

    if (pipe1 == -1 || pipe1Size == -1 || mutex1 == -1 || mutex2 == -1) {
        perror("shm_open\n");
        return -1;
    }
    //проверка правильности выполнения системных вызовов
    if (ftruncate(pipe1, getpagesize()) == -1) {
        perror("ftruncate error\n");
        return -1;
    }
    if (ftruncate(pipe1Size, sizeof(int)) == -1) {
        perror("ftruncate error\n");
        return -1;
    }
    if (ftruncate(mutex1, sizeof(pthread_mutex_t)) == -1) {
        perror("ftruncate error\n");
        return -1;
    }
    if (ftruncate(mutex2, sizeof(pthread_mutex_t)) == -1) {
        perror("ftruncate error\n");
        return -1;
    }
}