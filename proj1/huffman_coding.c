#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define DEFAULT_IN "completeShakespeare.txt"
#define DEFAULT_OUT "huffman.out"

#define ASCII_MAX 128

void freq_sort(int *freq, char *chars, int size);
void get_paths(int argc, char **argv, char **input_path, char **output_path);
void save_arg(char **dest, char *src);


int main(int argc, char **argv)
{
    char *input_path = NULL, *output_path = NULL;
    char ch;

    // frequencies[x] = frequency of ASCII character x (b10) in the input
    int frequencies[ASCII_MAX]  = { 0 };
    int characters[ASCII_MAX]   = { 0 };

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
    freq_sort(frequencies, characters, ASCII_MAX);

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
// the corresponding index of `chars`
// i.e. `freq[x]` = frequency of `chars[x]`
void freq_sort(int *freq, char *chars, int size) {
    int temp;
    
    for (int end = 0; end < size - 1; end++) {
        for (int i = 0; i < size - end - 1; i++) {
            if (freq[i] > freq[i+1]) {
                temp = freq[i];
                freq[i] = freq[i+1];
                freq[i+1] = temp;
            }
        }
    }
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