#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define DEFAULT_IN "completeShakespeare.txt"
#define DEFAULT_OUT "huffman.out"

#define ASCII_MAX 128

struct freq_node {
    unsigned char val;
    int freq;
    struct freq_node *prev, *next;
    struct freq_node *left, *right; 
} typedef freq_node;

freq_node* pq_create_node(int val);
freq_node* pq_push(freq_node* head, freq_node* new_node);
void pq_pop(freq_node** head, freq_node** pop);

FILE *get_file(char path[], char mode[]);
void get_paths(int argc, char **argv, char *input_path, char *output_path);
void print_bin(unsigned int val, int size);
void free_memory(freq_node *node);

void generate_huffman_codes(freq_node *root, unsigned int buff, int depth, unsigned int codes[], int code_lengths[]);


int main(int argc, char **argv)
{
    char input_path[128] = { 0 }, output_path[128] = { 0 };

    get_paths(argc, argv, input_path, output_path);

    // frequencies[x] = {val: x, freq: <frequency>}
    freq_node *frequencies[ASCII_MAX] = { 0 };

    for (int i = 0; i < ASCII_MAX; i++) {
        frequencies[i] = pq_create_node(0);
    }

    FILE *input = get_file(input_path, "r");

    // determine character frequencies
    char ch;
    while ((ch = fgetc(input)) != EOF) {
        if (frequencies[ch]->val == 0) {
            frequencies[ch]->val = ch; 
        }
        
        frequencies[ch]->freq++;
    }

    fclose(input);

    // push all frequencies into pq
    freq_node* pq_head = NULL;
    for (int i = 0; i < ASCII_MAX; i++) {
        if (frequencies[i] && frequencies[i]->val != 0) {
            pq_head = pq_push(pq_head, frequencies[i]);
        }
    }

    // build tree out of pq
    freq_node *current = pq_head;
    freq_node *left, *right;

    while (current->next != NULL) {
        // pop first two elements
        pq_pop(&current, &left);
        pq_pop(&current, &right);

        // create internal node
        freq_node *internal_node = pq_create_node('$');
        internal_node->freq = left->freq + right->freq;

        // assign children to internal node
        internal_node->left = left;
        internal_node->right = right;
        
        current = pq_push(current, internal_node);
    }

    int code_lengths[ASCII_MAX] = {0};
    unsigned int codes[ASCII_MAX] = {0};
    unsigned int buff = 0;
    
    generate_huffman_codes(current, buff, 0, codes, code_lengths);

    freq_node *tree_root = current;

    current = pq_head;

    // print codes (debug)
    printf("%-8s %-8s %-8s %s\n", "ascii", "freq", "codelen", "code");
    for (int i = 0; i < ASCII_MAX; i++) {
        if ( (ch = frequencies[i]->val) != 0) {
            printf("%-8c %-8d %-8u ", ch, frequencies[i]->freq, code_lengths[ch]);
            print_bin(codes[ch], code_lengths[ch]);
            printf("\n");
        }
    }

    // encode file
    input = get_file(input_path, "r");
    FILE *output = get_file(output_path, "wb");

    unsigned char byte_buffer = 0;
    unsigned int current_code;

    int buffer_index = 7;

    // a flag for keeping track of whether the buffer has been written to the file
    // this is needed to account for the last byte, which may not be written if it's
    // not full 
    int written = 0;

    while ((ch = fgetc(input)) != EOF) {
        written = 0;

        // get current character code
        current_code = codes[ch];

        // pack a byte
        for (int i = 1; i <= code_lengths[ch]; i++) {
            // write `length - i`th bit to byte at `buffer_index`
            byte_buffer |= (current_code >> (code_lengths[ch] - i) & 1) << buffer_index;

            // if the buffer is full, write it to the file
            if (buffer_index == 0) {
                putc(byte_buffer, output);

                buffer_index = 7;
                byte_buffer = 0;

                written = 1;
                continue;
            }

            buffer_index--;
        }
    }

    // if the last byte wasn't full it wasn't written
    // so, write here
    if (!written) putc(byte_buffer, output);

    fclose(input);
    fclose(output);

    free_memory(tree_root);

    return 0;
}

// free all tree nodes
void free_memory(freq_node *node) {
    if (node == NULL) return;

    if (node->left == NULL && node->right == NULL) {
        free(node);
        return;
    }

    free_memory(node->left);
    free_memory(node->right);
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

// helper for printing binary values
void print_bin(unsigned int val, int size) {
    for (int i = 1; i <= size; i++) {
        printf("%u", (val >> (size - i)) & 1);
    }
}

// traverse the tree to generate a code for each character
void generate_huffman_codes(freq_node *root, unsigned int buff, int depth, unsigned int codes[], int code_lengths[]) {
    if (root == NULL || root->val == 0) return;

    if (root->left == NULL && root->right == NULL) {
        codes[root->val] = buff;
        code_lengths[root->val] = depth;

        return;
    }

    depth++;

    // left = 1, right = 0
    generate_huffman_codes(root->left, (buff << 1) | 1,  depth, codes, code_lengths);
    generate_huffman_codes(root->right, (buff << 1),  depth, codes, code_lengths);
}

// helper function for allocating a new node
freq_node* pq_create_node(int val) {
    freq_node* new_node = malloc(sizeof(freq_node));

    new_node->val = val;
    new_node->freq = 0;
    new_node->prev = new_node->next = NULL;
    new_node->left = new_node->right = NULL;

    return new_node;
}

// pops out the first (minimum) node, saving it to `pop`
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

// process the command line options (or fall back to default values):
//      -i <path>: input path 
//      -o <path>: output path 
void get_paths(int argc, char **argv, char *input_path, char *output_path) {
    int opt;

    // check if input/output paths are given
    while ((opt = getopt(argc, argv, "i:o:")) != -1) {

        switch (opt) {
            case 'i':
                strcpy(input_path, optarg);
                break;

            case 'o':
                strcpy(output_path, optarg);
                break;

            default:
                printf("Usage: %s [-i input] [-o output]\n", argv[0]);
                exit(1);
        }

    }

    // use default values if no input
    if (input_path[0] == 0) {
        strcpy(input_path, DEFAULT_IN);
    }
    if (output_path[0] == 0) {
        strcpy(output_path, DEFAULT_OUT);
    }
}
