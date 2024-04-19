#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LOCK_SUFFIX ".lck"

volatile sig_atomic_t lock_count = 0;

void sigint_handler(int sig) {
    FILE *stats_file = fopen("stats.txt", "a");
    if (stats_file == NULL) {
        perror("Error opening stats file");
        exit(1);
    }
    fprintf(stats_file, "Process %d: %d locks\n", getpid(), lock_count);
    fclose(stats_file);
    exit(0);
}

int main(int argc, char *argv[]) {
    int opt;
    char *filename = NULL;

    while ((opt = getopt(argc, argv, "f:")) != -1) {
        switch (opt) {
            case 'f':
                filename = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s -f filename\n", argv[0]);
                exit(1);
        }
    }

    if (filename == NULL) {
        fprintf(stderr, "Usage: %s -f filename\n", argv[0]);
        exit(1);
    }

    signal(SIGINT, sigint_handler);

    char lockfile[strlen(filename) + strlen(LOCK_SUFFIX) + 1];
    sprintf(lockfile, "%s%s", filename, LOCK_SUFFIX);

    while (1) {
        int fd = open(lockfile, O_WRONLY | O_CREAT | O_EXCL, 0644);
        if (fd == -1) {
            usleep(100); // Sleep for 0.1 ms if lock file exists
            continue;
        }

        char pid_str[10];
        sprintf(pid_str, "%d", getpid());
        write(fd, pid_str, strlen(pid_str));
        close(fd);

        // Simulate file read/write
        sleep(1);

        fd = open(lockfile, O_RDONLY);
        if (fd == -1) {
            fprintf(stderr, "Error: Lock file disappeared\n");
            exit(1);
        }

        char pid_buf[10];
        read(fd, pid_buf, sizeof(pid_buf));
        close(fd);

        if (strcmp(pid_buf, pid_str) != 0) {
            fprintf(stderr, "Error: Lock file corrupted\n");
            exit(1);
        }

        if (unlink(lockfile) == -1) {
            perror("Error removing lock file");
            exit(1);
        }

        lock_count++;
    }

    return 0;
}