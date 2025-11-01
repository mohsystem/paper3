#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ORDERS 1024
#define MAX_STR 128

typedef struct {
    char username[MAX_STR];
    char stock_name[MAX_STR];
    int stock_quantity;
} Order;

static Order DATABASE[MAX_ORDERS];
static int DB_COUNT = 0;

char* process_buy_order(const char* stock_name, int stock_quantity, const char* username) {
    if (DB_COUNT < MAX_ORDERS) {
        strncpy(DATABASE[DB_COUNT].username, username, MAX_STR - 1);
        DATABASE[DB_COUNT].username[MAX_STR - 1] = '\0';
        strncpy(DATABASE[DB_COUNT].stock_name, stock_name, MAX_STR - 1);
        DATABASE[DB_COUNT].stock_name[MAX_STR - 1] = '\0';
        DATABASE[DB_COUNT].stock_quantity = stock_quantity;
        DB_COUNT++;
    }
    size_t len_username = strlen(username);
    size_t needed = strlen("/users/") + len_username + strlen("/profile") + 1;
    char* redirect = (char*)malloc(needed);
    if (!redirect) return NULL;
    snprintf(redirect, needed, "/users/%s/profile", username);
    return redirect;
}

int main() {
    char* r1 = process_buy_order("AAPL", 10, "alice");
    char* r2 = process_buy_order("GOOGL", 5, "bob");
    char* r3 = process_buy_order("TSLA", 2, "alice");
    char* r4 = process_buy_order("MSFT", 100, "carol");
    char* r5 = process_buy_order("AMZN", 7, "dave");

    if (r1) { printf("%s\n", r1); free(r1); }
    if (r2) { printf("%s\n", r2); free(r2); }
    if (r3) { printf("%s\n", r3); free(r3); }
    if (r4) { printf("%s\n", r4); free(r4); }
    if (r5) { printf("%s\n", r5); free(r5); }

    return 0;
}