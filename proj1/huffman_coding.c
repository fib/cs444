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
    struct freq_node *prev, *next;
    struct freq_node *parent, *left, *right; 
} typedef freq_node;

freq_node* pq_create_node(int val);
void pq_pop(freq_node** head, freq_node** pop);
freq_node* pq_push(freq_node* head, freq_node* new_node);
int cmp_freq_nodes(const void* a, const void* b);
void get_paths(int argc, char **argv, char *input_path, char *output_path);
void save_arg(char *dest, char *src);


int main(int argc, char **argv)
{
    char input_path[128] = { 0 }, output_path[128] = { 0 };

    // frequencies[x] = {val: x, freq: <frequency>}
    freq_node *frequencies[ASCII_MAX] = { 0 };

    get_paths(argc, argv, input_path, output_path);

    FILE *input = fopen(input_path, "r");

    // exit if input is invalid
    if (input == NULL) {
        printf("failed to open file: %s\n", input_path);
        exit(1);
    } 

    char ch;

    // printf("%lu\n", sizeof(freq_node));

    // frequencies['a'] = pq_create_node('a');
    // freq_node *head = NULL;

    // printf("%c: %d\n", frequencies['a']->val, frequencies['a']->freq);

    // head = pq_push(head, frequencies['a']);

    // printf("%c: %d\n", head->val, head->freq);

    // determine character frequencies
    do {
        ch = fgetc(input);

        if (frequencies[ch] == NULL) {
            frequencies[ch] = pq_create_node(ch); 
        }
        
        frequencies[ch]->freq++;
    } while (ch != EOF);

    fclose(input);

    freq_node* head = NULL;

    // // push all frequencies into pq
    // for (int i = 0; i < ASCII_MAX; i++) {
    //     printf("%d\n", i);
    //     if (frequencies[i]->val != 0) head = pq_push(head, frequencies[i]);
    // }

    // freq_node *curr = head;
    // freq_node *temp;

    // while (curr != NULL) {
    //     printf("%c: %d\n", curr->val, curr->freq);
    //     curr = curr->next;
    // }

    // // build tree
    // freq_node *current = head;
    // freq_node *left, *right;

    // while (current->next != NULL) {
    //     // pop first two elements
    //     pq_pop(&current, &left);
    //     pq_pop(&current, &right);

    //     printf("left: %c (%d), right: %c (%d)\n", left->val, left->freq, right->val, right->freq);

    //     // create internal node
    //     freq_node *internal_node = pq_create_node(0);
    //     internal_node->freq = left->freq + right->freq;

    //     internal_node->left = left;
    //     internal_node->right = right;
        
    //     left->parent = internal_node;
    //     right->parent = internal_node;

    //     printf("new node: %c (%d)\n\n", internal_node->val, internal_node->freq);

    //     current = pq_push(current, internal_node);
    // }

    // for (int i = 0; i < ASCII_MAX;)

    for (int i = 0; i < ASCII_MAX; i++) {
        if (frequencies[i] != NULL) {
            printf("freeing %c\n", frequencies[i]->val);
            free(frequencies[i]);
        }
    }


    return 0;
}

// helper function for allocating a new node
freq_node* pq_create_node(int val) {
    freq_node* new_node = malloc(sizeof(freq_node));

    new_node->val = val;
    new_node->freq = 0;
    new_node->prev = new_node->next = NULL;
    new_node->left = new_node->right = new_node->parent = NULL;

    return new_node;
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
