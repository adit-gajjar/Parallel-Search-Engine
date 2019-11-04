#ifndef WORKER_H
#define WORKER_H

#define PATHLENGTH 128
#define MAXRECORDS 100

#define MAXWORKERS 10

// This data structure is used by the workers to prepare the output
// to be sent to the master process.

typedef struct {
    int freq;
    char filename[PATHLENGTH];
} FreqRecord;

FreqRecord *get_word(char *word, Node *head, char **file_names);
void print_freq_records(FreqRecord *frp);
void run_worker(char *dirname, int in, int out);
void init_master_array(FreqRecord *master);
void add_to_master(FreqRecord *master, FreqRecord new_member);
void valid_sub_directory(char * dirname);

#endif /* WORKER_H */
