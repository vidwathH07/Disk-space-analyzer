#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>

long long calculateDiskSpace(char *path, long long minSize, long long maxSize);
void analyzeLastModifiedTime(char *path);

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 4) {
        printf("Usage: %s <directory> [min_size] [max_size]\n", argv[0]);
        printf("Optional: [min_size] and [max_size] to filter files within the specified range.\n");
        return 1;
    }

    char *path = argv[1];
    long long minSize = -1, maxSize = -1;

    if (argc >= 3) {
        minSize = atoll(argv[2]);
    }
    if (argc >= 4) {
        maxSize = atoll(argv[3]);
    }

    long long totalSpace = calculateDiskSpace(path, minSize, maxSize);
    printf("Total disk space used by %s", path);
    if (minSize != -1 || maxSize != -1) {
        printf(" within the size range ");
        if (minSize != -1) {
            printf(">= %lld bytes ", minSize);
        }
        if (maxSize != -1) {
            printf("and <= %lld bytes ", maxSize);
        }
    }
    printf(": %lld bytes\n", totalSpace);

    // Analyze last modified time
    analyzeLastModifiedTime(path);

    return 0;
}

long long calculateDiskSpace(char *path, long long minSize, long long maxSize) {
    struct stat statbuf;
    long long totalSpace = 0;

    if (lstat(path, &statbuf) == -1) {
        perror("lstat");
        exit(EXIT_FAILURE);
    }

    if (S_ISREG(statbuf.st_mode)) { // If it's a regular file
        if ((minSize == -1 || statbuf.st_size >= minSize) && (maxSize == -1 || statbuf.st_size <= maxSize)) {
            return statbuf.st_size;
        } else {
            return 0; // File not within size range
        }
    } else if (S_ISDIR(statbuf.st_mode)) { // If it's a directory
        DIR *dir;
        struct dirent *entry;

        dir = opendir(path);
        if (dir == NULL) {
            perror("opendir");
            exit(EXIT_FAILURE);
        }

        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char *newPath = malloc(strlen(path) + strlen(entry->d_name) + 2);
                sprintf(newPath, "%s/%s", path, entry->d_name);
                totalSpace += calculateDiskSpace(newPath, minSize, maxSize);
                free(newPath);
            }
        }

        closedir(dir);
    }

    return totalSpace;
}

void analyzeLastModifiedTime(char *path) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    // Array to store counts of files based on last modified time
    int counts[24] = {0}; // Count of files for each hour of the day

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char *newPath = malloc(strlen(path) + strlen(entry->d_name) + 2);
            sprintf(newPath, "%s/%s", path, entry->d_name);

            struct stat statbuf;
            if (stat(newPath, &statbuf) != -1) {
                time_t modTime = statbuf.st_mtime;
                struct tm *tm_info = localtime(&modTime);
                int hour = tm_info->tm_hour;
                counts[hour]++;
            }
            free(newPath);
        }
    }

    closedir(dir);

    // Display distribution of file counts based on last modified time
    printf("Distribution of file counts based on last modified time:\n");
    for (int i = 0; i < 24; i++) {
        printf("Hour %02d: %d files\n", i, counts[i]);
    }
}
