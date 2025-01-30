#ifndef INVENTORY_H
#define INVENTORY_H
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <sys/wait.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <time.h>

#define ITEMS 20 
#define PURCHASES 10 
#define CLIENTS 5

typedef struct {
    char description[50];
    float price;
    int item_count;
} inventory;

void createInventory(inventory items[]); 
void processOrder(inventory items[], int client_socket, int* revenue, int* successful_orders, int* failed_orders, int failure[][CLIENTS], int frequency[], int client_index); 

#endif // INVENTORY_H
