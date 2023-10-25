/*
diar.c: decode an array of 7 files (emulating RAID 2) using Hamming(7, 4).

Usage:
    ./diar -f filename (default: test.txt) -s size
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#define DEFAULT_IN "test.txt"

void init_raid2(FILE *raid2[7], char basename[128]);
unsigned char encode_nibble(unsigned char nibble); 

void get_args(int argc, char **argv, char *input_path, int *size);
FILE *get_file(char path[], char mode[]);


int main(int argc, char **argv) {
    char input_path[128] = { 0 }, output_path[128] = { 0 };
    int output_size = 0;
    FILE *output, *raid2[7];

    /* setup */

    get_args(argc, argv, input_path, &output_size);
    sprintf(output_path, "%s.2", input_path);

    output = get_file(output_path, "w");
    init_raid2(raid2, input_path);

    /* decoding */

    unsigned char hamming_code = 0;
    unsigned char p1, p2, d1, p3, d2, d3, d4;
    int parity_check = 0;

    // buffer for each output file
    unsigned char buffers[7] = { 0 };
    unsigned char output_buffer = 0;
    int buffer_index = -1;
    int nibble_counter = 0;

    for (int i = 0; i < output_size * 2; i++) {
        // if the current byte has been fully processed, read next byte from
        // each RAID file
        if (buffer_index < 0) {
            for (int j = 0; j < 7; j++) {
                buffers[j] = fgetc(raid2[j]);
            }
            buffer_index = 7;
        }

        hamming_code = 0;

        // assemble hamming code by grabbing 1 bit from each RAID file
        for (int j = 0; j < 7; j++) {
            hamming_code |= (buffers[j] >> buffer_index) & 1;
            hamming_code <<= 1;
        }

        hamming_code >>= 1;
        buffer_index--;

        p1 = hamming_code >> 6 & 1;
        p2 = hamming_code >> 5 & 1;
        d1 = hamming_code >> 4 & 1;
        p3 = hamming_code >> 3 & 1;
        d2 = hamming_code >> 2 & 1;
        d3 = hamming_code >> 1 & 1;
        d4 = hamming_code & 1;

        /* error detection */

        if (p1 != (d1 ^ d2 ^ d4)) parity_check += 1;
        if (p2 != (d1 ^ d3 ^ d4)) parity_check += 2;
        if (p3 != (d2 ^ d3 ^ d4)) parity_check += 4;

        /* error correction */

        switch (parity_check) {
            case 1:
                p1 = !p1;
                break;
            case 2:
                p2 = !p2;
                break;
            case 3:
                d1 = !d1;
                break;
            case 4:
                p3 = !p3;
                break;
            case 5:
                d2 = !d2;
                break;
            case 6:
                d3 = !d3;
                break;
            case 7:
                d4 = !d4;
                break;
        }

        /* assemble output buffer */

        if (nibble_counter == 0) {
            output_buffer = d1 << 7 | d2 << 6 | d3 << 5 | d4 << 4;
            nibble_counter++;
        } else if (nibble_counter == 1) {
            output_buffer |= d1 << 3 | d2 << 2 | d3 << 1 | d4;
            fputc(output_buffer, output);
            nibble_counter = 0;
        }
    }

    fclose(output);
    for (int i = 0; i < 7; i++) fclose(raid2[i]);

    return 0;
}

// initialize an array of 7 files representing 7 RAID 2 drives
void init_raid2(FILE *raid2[7], char basename[128]) {
    char output_path[128] = { 0 };

    for (int i = 0; i < 7; i++) {
        sprintf(output_path, "%s.part%d", basename, i);
        raid2[i] = get_file(output_path, "r");
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
void get_args(int argc, char **argv, char *input_path, int *size) {
    int opt;

    // check if input/output paths are given
    while ((opt = getopt(argc, argv, "f:s:")) != -1) {

        switch (opt) {
            case 'f':
                strcpy(input_path, optarg);
                break;
            case 's':
                *size = atoi(optarg);
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
