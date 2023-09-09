#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define DEFAULT_IN "completeShakespeare.txt"
#define DEFAULT_OUT "huffman.out"

void get_paths(int argc, char **argv, char **input_path, char **output_path);
void save_arg(char **dest, char *src);


int main(int argc, char **argv)
{
    char *input_path = NULL, *output_path = NULL;

    get_paths(argc, argv, &input_path, &output_path);

    printf("input: %s\n", input_path);
    printf("output: %s\n", output_path);

    // cleanup
    free(input_path);
    free(output_path);

    return 0;
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
    if (input_path == NULL) {
        save_arg(input_path, DEFAULT_IN);
    }
    if (output_path == NULL) {
        save_arg(output_path, DEFAULT_OUT);
    }
}

// helper function for allocating memory for strings
void save_arg(char **dest, char *src) {
    *dest = (char*)malloc(sizeof(char) * strlen(src));
    strcpy(*dest, src);
}