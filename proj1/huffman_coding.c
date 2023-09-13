#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define DEFAULT_IN "completeShakespeare.txt"
#define DEFAULT_OUT "huffman.out"

#define ASCII_MAX 128

struct pq_node {
    char c;
    int freq;
} typedef pq_node;

pq_node* freq_sort(int *freq, int size);
void get_paths(int argc, char **argv, char **input_path, char **output_path);
void save_arg(char **dest, char *src);


int main(int argc, char **argv)
{
    char *input_path = NULL, *output_path = NULL;
    char ch;

    // frequencies[x] = frequency of ASCII character x (b10) in the input
    int frequencies[ASCII_MAX]  = { 0 };

    get_paths(argc, argv, &input_path, &output_path);

    FILE *input = fopen(input_path, "r");

    // exit if input is invalid
    if (input == NULL) {
        printf("failed to open file: %s\n", input_path);
        exit(1);
    } 

    // determine character frequencies
    do {
     ch = fgetc(input);
     frequencies[ch]++;            
    } while (ch != EOF);

    for (int i = 0; i < 128; i++) {
        printf("%c: %d\n", i, frequencies[i]);
    }

    // sort characters by ascending frequency
    pq_node *freq_pq_head = freq_sort(frequencies, ASCII_MAX);

    // TODO:
    // 1. build frequency PQ
    // 2. build tree using the PQ
    // 3. encode string using tree

    // cleanup
    free(input_path);
    free(output_path);

    return 0;
}

// sort the frequencies array and save the relevant character in
pq_node* freq_sort(int *freq, int size) {
    pq_node freq_pq[ASCII_MAX] = { 0 };
}

// process the command line options (or fall back to default values):
//      -i <path>: input path 
//      -o <path>: output path 
void get_paths(int argc, char **argv, char **input_path, char **output_path) {
    int opt;

    // check if input/output paths are given
    while ((opt = getopt(argc, argv, "i:o:")) != -1) {

        switch (opt) {
            case 'i':
                save_arg(input_path, optarg);
                break;

            case 'o':
                save_arg(output_path, optarg);
                break;

            default:
                printf("Usage: %s [-i input] [-o output]\n", argv[0]);
                exit(1);
        }

    }

    // use default values if no input
    if (*input_path == NULL) {
        save_arg(input_path, DEFAULT_IN);
    }
    if (*output_path == NULL) {
        save_arg(output_path, DEFAULT_OUT);
    }
}

// helper function for allocating memory for strings
void save_arg(char **dest, char *src) {
    *dest = (char*)malloc(sizeof(char) * strlen(src));
    strcpy(*dest, src);
}