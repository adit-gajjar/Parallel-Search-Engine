#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

#include "freq_list.h"
#include "worker.h"



/*
A helper function used to find a word in a linkedlist of Nodes
if the word is found a pointer to the Node is returned, otherwise
NULL is returned.
*/
Node *find_word(char *word, Node *head){
    Node *curr = head;

    while(curr != NULL){
        if (strcmp(curr->word, word) == 0){
            return curr;
        }
        curr = curr->next;
    }

    return NULL;
}

/*
creates a FreqRecord struct with a given file name and frequency
*/
FreqRecord *create_freq_record(char *filename, int frequency){
    FreqRecord *freq_rec = malloc(sizeof(FreqRecord));
    freq_rec->freq = frequency;
    strcpy(freq_rec->filename, filename);
    return freq_rec;
}

int get_num_occurences(int *occurences){
    int num = 0;
    for (int i = 0; i < MAXFILES; i++){
        if (occurences[i] > 0){
            num += 1;
        }
    }
    return num;
}


/* Print to standard output the frequency records for a word.
* Use this for your own testing and also for query.c
*/
void print_freq_records(FreqRecord *frp) {
    int i = 0;

    while (frp != NULL && frp[i].freq != 0) {
        printf("%d    %s\n", frp[i].freq, frp[i].filename);
        i++;
    }
}

/* Complete this function for Task 2 including writing a better comment.
*/
void run_worker(char *dirname, int in, int out) {
    char test[MAXWORD];
    Node *head = malloc(sizeof(Node));
    char **filenames = init_filenames();
    char index_path[PATHLENGTH];
    strcpy(index_path, dirname);
    strcat(index_path, "/index");
    char filenames_path[PATHLENGTH];
    strcpy(filenames_path, dirname);
    strcat(filenames_path, "/filenames");
    read_list(index_path, filenames_path, &head, filenames);

    while ((read(in, test, MAXWORD)) > 0){
    
        int k = strlen(test);
        test[k-1] = '\0';
        FreqRecord *frp = get_word(test, head, filenames);
        int i = 0;

        while (frp != NULL && frp[i].freq != 0) {
            if (write(out, &frp[i], sizeof(FreqRecord)) == -1){
                perror("write error");
                exit(1);
            }
            i++;
        }
        // writes the sentinal value to let them know working is done outputting.
        if (write(out, &frp[i], sizeof(FreqRecord)) == -1){
            perror("error writing last element");
        }
    }
  
    return;
}

FreqRecord *get_word(char *word, Node *head, char **file_names) {
    // find if word exists.
    Node *curr_word = find_word(word, head);
    if (curr_word != NULL){ // if word exists get the freq array from the node and create structure;
        int *freq_list = curr_word->freq;
        int size = get_num_occurences(freq_list);
        int index = 0;
        FreqRecord *result = malloc(sizeof(FreqRecord) * (size+1));
        for (int i = 0; i < MAXFILES; i++){
            if (freq_list[i] > 0){
                result[index] = *(create_freq_record(file_names[i], freq_list[i]));
                index += 1;
            }
        }

        return result;
    } else {
        FreqRecord *not_found = malloc(sizeof(FreqRecord));
        not_found[0] = *(create_freq_record("", 0));

        return not_found;
    }
    return NULL;
}

/*
Used it initialize master array with numbers;
*/
void init_master_array(FreqRecord *master){
    for (int i = 0; i < MAXRECORDS; i ++){
        master[i] = *(create_freq_record("", 0));
    }
}


void add_to_master(FreqRecord *master, FreqRecord new_member){
    for (int i = 0; i < MAXRECORDS; i++){

        if (master[i].freq < new_member.freq){
            int entry_point = i;
            for (int j = MAXRECORDS -1; j > entry_point; j--){
                master[j] = master[j-1];
            }
            master[entry_point] = new_member;
            break;
            
        }
    }
}

void valid_sub_directory(char * dirname){
    // so given a path call fopen to see if the file name is valid.
    char * index_path;
    if ((index_path = malloc(sizeof(char) * PATHLENGTH)) == NULL){
        perror("malloc");
        exit(1);
    };

    FILE * index;
    FILE * filenames;
    strcpy(index_path, dirname);
    strcat(index_path, "/index");
    char *filenames_path;
    if ((filenames_path = malloc(sizeof(char) * PATHLENGTH)) == NULL){
        perror("malloc");
        exit(1);
    }
    strcpy(filenames_path, dirname);
    strcat(filenames_path, "/filenames");

    if ((index = fopen(index_path, "r")) == NULL){
        perror("invalid sub-directory provided");
        exit(1);
    }

    if (fclose(index)){
        perror("close error");
        exit(1);
    }

    if ((filenames = fopen(filenames_path, "r")) == NULL){
        perror("invalid sub-directory provided");
        exit(1);
    }

    if (fclose(filenames)){
        perror("close error");
        exit(1);
    }

}