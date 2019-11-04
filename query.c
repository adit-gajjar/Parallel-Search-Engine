#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "freq_list.h"
#include "worker.h"

#define MAXFILES 50
#define MAXWORD 32
#define MAXLINE 1024
#define PATHLENGTH 128

/* A program to model calling run_worker and to test it. Notice that run_worker
 * produces binary output, so the output from this program to STDOUT will 
 * not be human readable.  You will need to work out how to save it and view 
 * it (or process it) so that you can confirm that your run_worker 
 * is working properly.
 */
int main(int argc, char **argv) {
    char ch;
    char path[PATHLENGTH];
    char *startdir = ".";
    char paths[10][PATHLENGTH];

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

    /* For each entry in the directory, eliminate . and .., and check
     * to make sure that the entry is a directory, then call run_worker
     * to process the index file contained in the directory.
     * Note that this implementation of the query engine iterates
     * sequentially through the directories, and will expect to read
     * a word from standard input for each index it checks.
     */
    int count = 0;
    struct dirent *dp;
    while ((dp = readdir(dirp)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 ||
            strcmp(dp->d_name, "..") == 0 ||
            strcmp(dp->d_name, ".svn") == 0 ||
            strcmp(dp->d_name, ".git") == 0) {
                continue;
        }

        strncpy(path, startdir, PATHLENGTH);
        strncat(path, "/", PATHLENGTH - strlen(path));
        strncat(path, dp->d_name, PATHLENGTH - strlen(path));
        path[PATHLENGTH - 1] = '\0';
        struct stat sbuf;
        if (stat(path, &sbuf) == -1) {
            // This should only fail if we got the path wrong
            // or we don't have permissions on this entry.
            perror("stat");
            exit(1);
        }

        // Only call run_worker if it is a directory
        // Otherwise ignore it.
        if (S_ISDIR(sbuf.st_mode)) {
             if (count == MAXWORKERS){
                fprintf(stderr, "Over 10 sub-directories");
                exit(1);
            } else {
                valid_sub_directory(path);
                strcpy(paths[count], path);
                count += 1;
            }
        }
    }

    // create a helper function which checks if the sub directory has the correct index and filenames file.

    // now set up the pipes and fork the processes.
    FreqRecord master[MAXRECORDS];
    int wfd[count][2];
    int rfd[count][2];
    // create a pipe
    for (int i = 0; i < count; i++){
        if (pipe(wfd[i]) == -1) {
            perror("wfd pipe creation.");
            exit(1);
        }

        if (pipe(rfd[i]) == -1) {
            perror("rfd pipe creation.");
            exit(1);
        }

    }


    for (int i = 0; i < count; i++){
        int r = fork();

        if (r == 0) {

            for (int j = 0; j < count; j++){
                if (close(wfd[j][1]) == -1){
                    perror("close");
                    exit(1);
                }
                if (close(rfd[j][0]) == -1){ // child write to rfd, reads from wfd.
                    perror("close");
                    exit(1);
                }
            }
        
            // Child will read from parent
            run_worker(paths[i], wfd[i][0], rfd[i][1]);

            
            if (close(wfd[i][0]) == -1){
                perror("close");
                exit(1);
            }
            if (close(rfd[i][1]) == -1){
                perror("close");
                exit(1);
            }
            exit(0);

        } else {
            if (close(wfd[i][0]) == -1){
                perror("close");
                exit(1);
            }
            if (close(rfd[i][1]) == -1){
                perror("close");
                exit(1);
            }
        }
    }
    char word[MAXWORD];
    init_master_array(master);
    // read the write file as fd2 is used to read from the child.
    while (fgets(word, MAXWORD, stdin) != NULL) {
         // making sure empty string isn't entered.
            for (int i = 0; i < count; i++){
                if (write(wfd[i][1], word, MAXWORD) == -1) {
                    perror("write to pipe");
                    exit(1);
                }
                

            }
            FreqRecord *from_child = malloc(sizeof(FreqRecord));
            for (int i = 0; i < count; i++){
                while (read(rfd[i][0], from_child, sizeof(FreqRecord)) == sizeof(FreqRecord)){
                    if (from_child->freq > 0){
                        add_to_master(master, *from_child);
            
                    }
                    else{ // we got the sentinal so no more read calls
                        break;
                    }
                }
            }
        // print our results.
        print_freq_records(master);
        // clear master array.
        init_master_array(master);
        

        // we need to wait until we get something to read from the hc
    }
    // for keeping it from hanging on the read call.

    // we can put this into a helper function.
    for (int i = 0; i < count; i++){
        if (close(wfd[i][1]) == -1){
            perror("close");
            exit(1);
        }

        wait(NULL);

    }
    if (closedir(dirp) < 0)
        perror("closedir");




    return 0;
}
