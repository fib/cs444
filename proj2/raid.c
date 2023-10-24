/*
raid.c: encode a file using Hamming(7, 4) and write it across 7 files (emulating RAID 2).

Usage:
    ./raid -f filename (default: test.txt)
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#define DEFAULT_IN "test.txt"

void init_raid2(FILE *raid2[7], char basename[128]);
unsigned char encode_nibble(unsigned char nibble); 

void get_arg_paths(int argc, char **argv, char *input_path);
FILE *get_file(char path[], char mode[]);


int main(int argc, char **argv) {
    char ch, input_path[128] = { 0 };
    FILE *input, *raid2[7];

    /* setup */

    get_arg_paths(argc, argv, input_path);

    input = get_file(input_path, "r");
    init_raid2(raid2, input_path);

    /* encoding */

    // buffer for each output file
    unsigned char buffers[7] = { 0 };
    int buffer_index = 0;

    // buffer for each nibble of a byte
    unsigned char nibbles[2] = { 0 };

    // read a byte from input
    while ((ch = fgetc(input)) != EOF) {
        nibbles[0] = ch >> 4;
        nibbles[1] = ch & 15;

        // for each nibble (2 per byte)
        for (int n = 0; n < 2; n++) {
            // write each bit of the encoded nibble to its corresponding file buffer
            for (int i = 0; i < 7; i++) {
                buffers[i] |= (encode_nibble(nibbles[n]) >> (6 - i) & 1) << (7 - buffer_index);
            }

            buffer_index++;

            // if buffers are full, write them to file & reset them
            if (buffer_index == 8) {
                for (int i = 0; i < 7; i++) {
                    putc(buffers[i], raid2[i]);
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

// encode a nibble using Hamming(7,4)
unsigned char encode_nibble(unsigned char nibble) {
    unsigned char encoded_nibble = 0;
    unsigned char d1 = nibble >> 3 & 1, d2 = nibble >> 2 & 1, d3 = nibble >> 1 & 1, d4 = nibble & 1;

    encoded_nibble |= (d1 ^ d2 ^ d4) << 6;      // p1
    encoded_nibble |= (d1 ^ d3 ^ d4) << 5;      // p2
    encoded_nibble |= d1 << 4;                  // d1
    encoded_nibble |= (d2 ^ d3 ^ d4) << 3;      // p3
    encoded_nibble |= d2 << 2;                  // d2
    encoded_nibble |= d3 << 1;                  // d3
    encoded_nibble |= d4;                       // d4

    return encoded_nibble;
}

// initialize an array of 7 files representing 7 RAID 2 drives
void init_raid2(FILE *raid2[7], char basename[128]) {
    char output_path[128] = { 0 };

    for (int i = 0; i < 7; i++) {
        sprintf(output_path, "%s.part%d", basename, i);
        raid2[i] = get_file(output_path, "w");
    }
}

// helper for accessing and validating files, exits on error
FILE *get_file(char path[], char mode[]) {
    FILE *file = fopen(path, mode);

    if (file == NULL) {
        printf("Failed to open file: %s\n", path);
        exit(1);
    }

    return file;
}

// process the command line options (or fall back to default values):
//      -f <path>: input file
void get_arg_paths(int argc, char **argv, char *input_path) {
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
