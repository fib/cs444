// count the frequency of characters in a string

int FREQUENCIES[128] = { 0 };

int freq_count(char c) {
    FREQUENCIES[c]++;
}