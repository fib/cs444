#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define DEFAULT_IN "completeShakespeare.txt"
#define DEFAULT_OUT "huffman.out"

#define ASCII_MAX 128

struct freq_node {
    char val;
    int freq;
    struct freq_node* prev;
    struct freq_node* next;
    struct freq_node* left; 
    struct freq_node* right; 
    struct freq_node* parent;
} typedef freq_node;

void pq_pop(freq_node** head, freq_node** pop);
freq_node* pq_push(freq_node* head, freq_node* new_node);
int cmp_freq_nodes(const void* a, const void* b);
void get_paths(int argc, char **argv, char *input_path, char *output_path);
void save_arg(char *dest, char *src);


int main(int argc, char **argv)
{
    char input_path[128] = { 0 }, output_path[128] = { 0 };

    // frequencies[x] = {val: x, freq: <frequency>}
    freq_node frequencies[ASCII_MAX] = { 0 };

    get_paths(argc, argv, input_path, output_path);

    FILE *input = fopen(input_path, "r");

    // exit if input is invalid
    if (input == NULL) {
        printf("failed to open file: %s\n", input_path);
        exit(1);
    } 

    char ch;

    // determine character frequencies
    do {
        ch = fgetc(input);
        frequencies[ch].val = ch;
        frequencies[ch].freq++;
    } while (ch != EOF);
    
    int count = 0;

    // move characters with non-zero frequency to the front of the array
    for (int i = 0; i < ASCII_MAX; i++) {
        if (frequencies[i].freq != 0) {
            frequencies[count++] = frequencies[i];
        }
    }

    // any elements past this index are to be ignored
    // this is treated as the new array size
    int number_of_unique_characters = count;
    
    freq_node* head = NULL;

    for (int i = 0; i < number_of_unique_characters; i++) {
        head = pq_push(head, &frequencies[i]);
    }

    freq_node *curr = head;
    freq_node *temp;

    while (curr != NULL) {
        pq_pop(&curr, &temp);
        printf("%c: %d\n", temp->val, temp->freq);
    }

    return 0;
}

void pq_pop(freq_node** head, freq_node** pop) {
    *pop = *head;

    *head = (*head)->next;

    if ((*head) != NULL) {

        (*head)->prev = NULL;
    }

    (*pop)->next = NULL;
}

// insert a new node at the correct position based on frequency
freq_node* pq_push(freq_node* head, freq_node* new_node) {
    freq_node* current_node = head;

    // if the pq is empty, set head as new_node
    if (current_node == NULL) {
        return new_node;
    // if the new_node should be inserted before the current head
    } else if (new_node->freq < current_node->freq) {
        new_node->next = current_node;
        new_node->prev = NULL;
        
        current_node->prev = new_node;

        return new_node;
    } else {
        // iterating up to the insertion point
        while (current_node->next != NULL && current_node->next->freq < new_node->freq) {
            current_node = current_node->next;
        }

        // if new_node is to be inserted at the end of the pq
        if (current_node== NULL) {
            current_node->next = new_node;
            new_node->prev = current_node;
            new_node->next = NULL;
        } else {
            new_node->prev = current_node;
            new_node->next = current_node->next;

            current_node->next = new_node;

            if (new_node->next != NULL) {
                new_node->next->prev = new_node;
            }
        }
    }

    return head;
}

// a comparison function comparing the frequencies of 2 pq nodes
int cmp_freq_nodes(const void* a, const void* b) {
    return ((freq_node*)a)->freq - ((freq_node*)b)->freq;
}

// process the command line options (or fall back to default values):
//      -i <path>: input path 
//      -o <path>: output path 
void get_paths(int argc, char **argv, char *input_path, char *output_path) {
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
    if (input_path[0] == 0) {
        save_arg(input_path, DEFAULT_IN);
    }
    if (output_path[0] == 0) {
        save_arg(output_path, DEFAULT_OUT);
    }
}

// helper function for allocating memory for strings
void save_arg(char *dest, char *src) {
    strcpy(dest, src);
}