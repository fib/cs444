/*
raid.c: encode a file using Hamming(7, 4) and write it to 7 files (emulating RAID 2).

Usage:
    ./raid -f filename
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#define DEFAULT_IN "test.txt"

unsigned char encode_nibble(unsigned char nibble); 

void get_paths(int argc, char **argv, char *input_path);
FILE *get_file(char path[], char mode[]);

int main(int argc, char **argv) {
    char ch;
    char input_path[128] = { 0 };
    char output_path[128] = { 0 };

    get_paths(argc, argv, input_path);

    FILE *input = get_file(input_path, "r");

    FILE *raid2[7] = { 0 };

    // initialize RAID 2 files
    for (int i = 0; i < 7; i++) {
        sprintf(output_path, "%s.part%d", input_path, i);
        raid2[i] = get_file(output_path, "w");
    }

    // contains the two nibbles of each character being read
    unsigned char nibbles[2];

    // stores the buffer for each bit in the hamming code, i.e.
    // buffers[0] = p1, buffers[1] = p2, buffers[2] = d1, etc.
    unsigned char buffers[7] = { 0 };
    int buffer_index = 0;

    while ((ch = fgetc(input)) != EOF) {
        nibbles[0] = ch >> 4;
        nibbles[1] = ch & 15;

        for (int n = 0; n < 2; n++) {

            for (int i = 0; i < 7; i++) {
                buffers[i] |= (encode_nibble(nibbles[n]) >> (6 - i) & 1) << (7 - buffer_index);
            }
            buffer_index++;

            // once buffers are full, write them to file
            if (buffer_index == 8) {
                for (int i = 0; i < 7; i++) {
                    putc(buffers[i], raid2[i]);

                    // reset buffer
                    buffers[i] = 0;
                }

                buffer_index = 0;
            }
        }
    }

    // close all files
    fclose(input);
    for (int i = 0; i < 7; i++) fclose(raid2[i]);

    return 0;
}

unsigned char encode_nibble(unsigned char nibble) {
    unsigned char encoded_nibble = 0;
    // p1
    encoded_nibble |= ((nibble >> 3 & 1) ^ (nibble >> 2 & 1) ^ (nibble & 1)) << 6;
    // p2
    encoded_nibble |= ((nibble >> 3 & 1) ^ (nibble >> 1 & 1) ^ (nibble & 1)) << 5;
    // d1
    encoded_nibble |= (nibble >> 3 & 1) << 4;
    // p3
    encoded_nibble |=  ((nibble >> 2 & 1) ^ (nibble >> 1 & 1) ^ (nibble & 1)) << 3;
    // d2
    encoded_nibble |= (nibble >> 2 & 1) << 2;
    // d3
    encoded_nibble |= (nibble >> 1 & 1) << 1;
    // d4
    encoded_nibble |= (nibble & 1);

    return encoded_nibble;
}

// help for accessing and validating files, exits on error
FILE *get_file(char path[], char mode[]) {
    FILE *file = fopen(path, mode);

    if (file == NULL) {
        printf("Failed to open file: %s\n", path);
        exit(1);
    }

    return file;
}

// process the command line options (or fall back to default values):
//      -f <path>: 
void get_paths(int argc, char **argv, char *input_path) {
    int opt;

    // check if input/output paths are given
    while ((opt = getopt(argc, argv, "f:")) != -1) {

        switch (opt) {
            case 'f':
                strcpy(input_path, optarg);
                break;

            default:
                printf("Usage: %s [-f filename]\n", argv[0]);
                exit(1);
        }

    }

    // use default values if no input
    if (input_path[0] == 0) {
        strcpy(input_path, DEFAULT_IN);
    }
}
