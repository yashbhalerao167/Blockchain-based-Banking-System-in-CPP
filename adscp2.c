#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Block {
    int index;
    char transactionID[65];
    char previousHash[65];
    char timestamp[64];
    char data[1024];
    char hash[65];
    struct Block *next;
} Block;

Block *blockchain = NULL;

void loadTransactionsFromCSV(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Failed to open file %s\n", filename);
        return;
    }

    char line[2048];
    fgets(line, sizeof(line), file); // Skip header
    Block *lastBlock = NULL;

    while (fgets(line, sizeof(line), file) != NULL) {
        Block *newBlock = (Block *)malloc(sizeof(Block));
        sscanf(line, "%d,%[^,],%[^,],%[^,],%[^,],%s",
               &newBlock->index, newBlock->transactionID, newBlock->previousHash,
               newBlock->timestamp, newBlock->data, newBlock->hash);
        newBlock->next = NULL;

        if (lastBlock != NULL) {
            lastBlock->next = newBlock;
        } else {
            blockchain = newBlock;
        }
        lastBlock = newBlock;
    }
    fclose(file);
}

void printBlockchain() {
    printf("Blockchain Visualization:\n\n");
    Block *current = blockchain;
    while (current != NULL) {
        printf("Block Index: %d\n", current->index);
        printf("Transaction ID: %s\n", current->transactionID);
        printf("Previous Hash: %s\n", current->previousHash);
        printf("Timestamp: %s\n", current->timestamp);
        printf("Data: %s\n", current->data);
        printf("Hash: %s\n", current->hash);
        printf("\n-------------------------------------\n");
        current = current->next;
    }
}

int main() {
    const char *transactionsFilename = "D:\\YASH\\COLLEGE\\ADS\\CP\\Banking System using Blockchain\\transactions.csv";
    loadTransactionsFromCSV(transactionsFilename);
    printBlockchain();
    return 0;
}
