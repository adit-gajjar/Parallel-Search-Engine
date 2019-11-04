// Function is used to test the get_word function
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "freq_list.h"
#include "worker.h"
int main(int argc, char **argv) {
    char ch;
    char *startdir = ".";

    /* this models using getopt to process command-line flags and arguments */
    while ((ch = getopt(argc, argv, "d:")) != -1) {
        switch (ch) {
        case 'd':
            startdir = optarg;
            break;
        default:
            fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME]\n");
            exit(1);
        }
    }

    // Open the directory provided by the user (or current working directory)
    DIR *dirp;
    if ((dirp = opendir(startdir)) == NULL) {
        perror("opendir");
        exit(1);
    }

    // open the index and files names and take user input to print the output.
    char index_path[PATHLENGTH];
    strcpy(index_path, startdir);
    strcat(index_path, "/index");
    char filenames_path[PATHLENGTH];
    strcpy(filenames_path, startdir);
    strcat(filenames_path, "/filenames");
    Node *head = malloc(sizeof(Node));
    char **filenames = init_filenames();

    read_list(index_path, filenames_path, &head, filenames);
    char input[MAXWORD];
    while (fgets(input, MAXWORD, stdin) != NULL){
        int k = strlen(input);
        input[k-1] = '\0';
        print_freq_records(get_word(input, head, filenames));
    }



    if (closedir(dirp) < 0)
        perror("closedir");

    return 0;
}