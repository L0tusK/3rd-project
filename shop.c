#include "inventory.h"

// Δημιουργία καταλόγου προϊόντων
void createInventory(inventory items[]) {
    for (int i = 0; i < ITEMS; i++) {
        // Δημιουργία περιγραφής για κάθε προϊόν
        sprintf(items[i].description, "Item %d", i + 1);
        items[i].price = rand() % 100 + 1;
        items[i].item_count = 2;  // Αρχικό διαθέσιμο απόθεμα για κάθε προϊόν
    }
}

// Χειρισμός παραγγελιών από πελάτη
void processOrder(inventory items[], int client_socket, int* revenue, int* successful_orders, int* failed_orders, int failure[][CLIENTS], int frequency[], int client_index) {
    for (int i = 0; i < PURCHASES; i++) {
        int product;
        read(client_socket, &product, sizeof(int)); // Διαβάζει τον κωδικό του προϊόντος από τον πελάτη
        usleep(500000);  // Μικρή καθυστέρηση για να προσομοιωθεί ο χρόνος επεξεργασίας

        int found = 0;
        char message[130];
        for (int j = 0; j < ITEMS; j++) {
            if (j + 1 == product) {
                frequency[j]++; // Αύξηση του πλήθους των ζητήσεων για το συγκεκριμένο προϊόν
                if (items[j].item_count > 0) { // Έλεγχος αν το προϊόν είναι διαθέσιμο
                    items[j].item_count--; // Μείωση του αποθέματος του προϊόντος
                    (*successful_orders)++; // Αύξηση των επιτυχημένων παραγγελιών
                    *revenue += items[j].price; // Προσθήκη της τιμής του προϊόντος στα έσοδα
                    sprintf(message, "Order %d for %s was successful\n", product, items[j].description);
                    write(client_socket, message, strlen(message) + 1);
                } else {
                    // Αν δεν υπάρχει διαθέσιμο προϊόν, αύξηση των αποτυχημένων παραγγελιών
                    (*failed_orders)++;
                    failure[j][client_index]++;
                    sprintf(message, "Order %d for %s failed\n", product, items[j].description);
                    write(client_socket, message, strlen(message) + 1);
                }
                found = 1;
                break;
            }
        }
        // Αν ο πελάτης ζήτησε λάθος κωδικό προϊόντος
        if (!found) {
            sprintf(message, "Invalid product code: %d\n", product);
            write(client_socket, message, strlen(message) + 1);
        }
    }
}

int main() {
    srand(time(NULL)); // Αρχικοποίηση τυχαίου αριθμού
    inventory items[ITEMS];
    createInventory(items); 

    int server_fd = socket(AF_INET, SOCK_STREAM, 0); // Δημιουργία του server
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 5);

    int successful_orders = 0, failed_orders = 0, revenue = 0;
    int failure[ITEMS][CLIENTS];
    memset(failure, 0, sizeof(failure));
    int frequency[ITEMS];
    memset(frequency, 0, sizeof(frequency));

    // Δημιουργία πελατών με χρήση fork
    for (int j = 0; j < CLIENTS; j++) {
        pid_t pid = fork();
        if (pid == 0) { // Παιδική διεργασία - πελάτης
            sleep(1); // Καθυστέρηση για να είναι έτοιμος ο server
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(8080);
            inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

            // Σύνδεση με τον server
            if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                perror("Connection failed");
                exit(EXIT_FAILURE);
            }

            // Ο πελάτης κάνει παραγγελίες
            for (int i = 0; i < PURCHASES; i++) {
                sleep(1); // 1 δευτερόλεπτο μεταξύ παραγγελιών
                int product = rand() % ITEMS + 1; 
                write(sock, &product, sizeof(int));
                char response[130];
                read(sock, response, sizeof(response));
                printf("Client %d received: %s", j + 1, response);
            }
            close(sock);
            exit(0);
        } else { // Γονική διεργασία - server
            int client_socket;
            struct sockaddr_in client_addr;
            socklen_t client_addrlen = sizeof(client_addr);
            client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_addrlen);
            processOrder(items, client_socket, &revenue, &successful_orders, &failed_orders, failure, frequency, j);
            close(client_socket);
            wait(NULL); 
        }
    }

    printf("\n"); // Εκτύπωση αποτελεσμάτων
    for (int i = 0; i < ITEMS; i++) {
        printf("%s was requested: %d times\n", items[i].description, frequency[i]);
        printf("and was sold: %d times\n", 2 - items[i].item_count);
        for (int j = 0; j < CLIENTS; j++) {
            if (failure[i][j] > 0) {
                printf("Client %d failed %d times for %s\n", j + 1, failure[i][j], items[i].description);
            }
        }
        printf("\n");
    }
    printf("Total successful orders: %d\n", successful_orders);
    printf("Total failed orders: %d\n", failed_orders);
    printf("Total revenue: %.2f\n", (float)revenue);
    printf("\n");

    close(server_fd);
    return 0;
}
