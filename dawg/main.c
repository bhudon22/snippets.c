#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "colors.h"

//#define SIZE 30               // stack info - might not use
#define true 1                  // define bool values
#define false 0
typedef char bool;

#define SIZE 26

typedef struct TrieNode TrieNode;   // advanced definition
// Arena for our dawg array - only manage one malloc
typedef struct{
    uint8_t* data;
    size_t size;
    size_t offset;
} Arena;

struct TrieNode{
    unsigned int numNode;           // for our use only
    char letter;                    // char character - again for printing the dawg
    TrieNode* children[SIZE];       // array for all available letters
    bool isEndOfWord;               //is a valid word
};

char* loadFile();
bool validateBuffer(char* buffer);
Arena createArena(size_t size);
void* arenaAlloc(Arena* arena, size_t size);
void freeArena(Arena arena);
TrieNode* createNode(Arena *arena);
void insert(Arena* arena, TrieNode *root, const char *key);
void printDawg(TrieNode *root);
void listDawg(TrieNode *root, char *buffer, int wordIndex);



//bool search(struct TrieNode *root, const char *key);
//bool isempty(struct TrieNode *root) ;
//struct TrieNode* deleteHelper(struct TrieNode *root, const char *key, int depth);
//void deletekey(struct TrieNode *root, const char *key);


//void printNodes(struct TrieNode *root);
//void push(struct TrieNode *ptr);
//struct TrieNode* pop();
//void pop();
//void show();

int numWords=0,numNodes = 0;        // number of allocated nodes and words
//char *wordArray[40];                // array to hold words
//int wordIndex=0;                    // index position in wordarray

int main(void) {

    char *buffer = loadFile();
    if (buffer == NULL) {
        printf("Error Loading file.../n");
        return 1;
    }

    if (validateBuffer(buffer)==false) {
        printf("Error Validating file.../n");
        return 1;
    }

    // Create Arena
    Arena arena = createArena(sizeof(struct TrieNode)*300);
    //printf("Size of Arena: %ld\n",arena.size);

    // Create root node
    TrieNode *root = createNode(&arena);

    // Returns first line
    char* line = strtok(buffer, "\n");
    // Keep printing tokens while one of the
    // delimiters present in str[].
    while (line != NULL) {
        numWords++;
        printf("%s\n", line);
        insert(&arena,root, line);

        line = strtok(NULL, "\n");
    }

    printDawg(root);
    printf("\n");


    int wordIndex=0;
    char wordBuffer[40];
    listDawg(root,wordBuffer,wordIndex);



    red();
    printf("Hello, World!  char=%c\n",root->letter);
    reset();

    freeArena(arena);
    free(buffer);
    return 0;
}

char* loadFile() {
    //FILE *file = fopen("scrabble_words.txt", "rb");  // open file in binary mode
    FILE *file = fopen("test.txt", "rb");  // open file in binary mode


    if (file == NULL) {
        perror("file open error");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    const long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(file_size + 1);

    if (buffer == NULL) {
        perror("malloc error");
        fclose(file);
        return NULL;
    }

    const size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
    if (bytes_read != file_size) {
        perror("fread error - bytes read");
        fclose(file);
        return NULL;
    }
    fclose(file);
    buffer[file_size] = '\0'; // null terminate array
    //printf("filesize=%ld\n",file_size);
    return buffer;
}

bool validateBuffer(char* buffer) {
    long index = 0;
    char *c = buffer;
    while (*c++) index++;           // ugly but  compact - scan buffer and find 0 return index

    bool flag = true;
    for (int i=0;i<index;i++) {                 // scan all chars in buffer
        char ch = buffer[i];
        if (ch=='\n') continue;                 // ignore eol
        if((ch >= 'a' && ch <= 'z' )) continue; // ignore lowercase
        flag = false;                           // undefined char found
    }
    return flag;
}

Arena createArena(size_t size) {
    Arena arena;
    arena.data  = malloc(size);
    arena.size = size;
    arena.offset =0;
    return arena;
}

void* arenaAlloc(Arena* arena, size_t size) {
    if (arena->offset + size > arena->size) {
        printf("ERROR - Arena out of space");
        return NULL;  //Arena is full
    }
    void* ptr = arena->data+arena->offset;
    arena->offset += size;
    return ptr;
}

void freeArena(Arena arena) {
    free(arena.data);
}


TrieNode *createNode(Arena *arena) {
    //struct TrieNode *node = (struct TrieNode *)malloc(sizeof(struct TrieNode));
    TrieNode *node = (TrieNode *)arenaAlloc(arena,sizeof(struct TrieNode));
    node->isEndOfWord = false;
    node->letter="";
    if (numNodes==0) node->letter='*';      //numNodes==0 then it's root letter = *
    for (int i = 0; i < SIZE; i++) {
        node->children[i] = NULL;
    }
    numNodes++;                         //increment number of allocated nodes
    node->numNode = numNodes;           // tag the node for out convenience

    return node;
}

void insert(Arena *arena, TrieNode *root, const char *key) {
    TrieNode *current = root;
    for (int i = 0; i < strlen(key); i++) {
        int index = key[i] - 'a';
        if (current->children[index] == NULL) {
            current->children[index] = createNode(arena);
            current->children[index]->letter = key[i];
        }
        current = current->children[index];
    }
    //printf("-%s",key);
    current->isEndOfWord = true;
}

void printDawg(TrieNode* root) {
    // Prints the nodes of the trie
    if (!root) return;              // if root == NULL (0) then don't bother
    TrieNode* temp = root;
    if (temp->isEndOfWord==true) {
        green();
        printf("%c->", temp->letter);
        reset();
    } else  printf("%c->", temp->letter);
    for (int i=0; i<SIZE; i++) {
        if (temp->children[i]) {            // ignore NULL
            printDawg(temp->children[i]);
        }
    }
}

void listDawg(TrieNode* root,char *buffer, int wordIndex) {
    if (!root) return;              // if root == NULL (0) then don't bother
    for (int i = 0;i<SIZE;i++) {
        if(root->children[i] != NULL) {
            buffer[wordIndex] = i + 'a';
            if(root->children[i]->isEndOfWord == true) {
                buffer[wordIndex + 1] = '\0';
                printf("%s\n",buffer);
            }
            listDawg(root->children[i], buffer, wordIndex + 1);
        }
    }
}