#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct {
    long long balance;
    bool isOpen;
    pthread_mutex_t mutex;
} BankAccount;

// Function return codes
#define SUCCESS 0
#define ERROR_ACCOUNT_CLOSED -1
#define ERROR_INSUFFICIENT_FUNDS -2
#define ERROR_INVALID_ARGUMENT -3
#define ERROR_ALREADY_OPEN -4

BankAccount* openAccount(long long initialBalance) {
    if (initialBalance < 0) {
        return NULL;
    }
    BankAccount* acc = (BankAccount*)malloc(sizeof(BankAccount));
    if (acc == NULL) {
        return NULL;
    }
    acc->balance = initialBalance;
    acc->isOpen = true;
    if (pthread_mutex_init(&acc->mutex, NULL) != 0) {
        free(acc);
        return NULL;
    }
    return acc;
}

int closeAccount(BankAccount* acc) {
    pthread_mutex_lock(&acc->mutex);
    if (!acc->isOpen) {
        pthread_mutex_unlock(&acc->mutex);
        return ERROR_ACCOUNT_CLOSED;
    }
    acc->isOpen = false;
    pthread_mutex_unlock(&acc->mutex);
    return SUCCESS;
}

void destroyAccount(BankAccount* acc) {
    if (acc) {
        pthread_mutex_destroy(&acc->mutex);
        free(acc);
    }
}

int getBalance(BankAccount* acc, long long* balance) {
    pthread_mutex_lock(&acc->mutex);
    if (!acc->isOpen) {
        pthread_mutex_unlock(&acc->mutex);
        return ERROR_ACCOUNT_CLOSED;
    }
    *balance = acc->balance;
    pthread_mutex_unlock(&acc->mutex);
    return SUCCESS;
}

int deposit(BankAccount* acc, long long amount) {
    if (amount <= 0) {
        return ERROR_INVALID_ARGUMENT;
    }
    pthread_mutex_lock(&acc->mutex);
    if (!acc->isOpen) {
        pthread_mutex_unlock(&acc->mutex);
        return ERROR_ACCOUNT_CLOSED;
    }
    acc->balance += amount;
    pthread_mutex_unlock(&acc->mutex);
    return SUCCESS;
}

int withdraw(BankAccount* acc, long long amount) {
    if (amount <= 0) {
        return ERROR_INVALID_ARGUMENT;
    }
    pthread_mutex_lock(&acc->mutex);
    if (!acc->isOpen) {
        pthread_mutex_unlock(&acc->mutex);
        return ERROR_ACCOUNT_CLOSED;
    }
    if (acc->balance < amount) {
        pthread_mutex_unlock(&acc->mutex);
        return ERROR_INSUFFICIENT_FUNDS;
    }
    acc->balance -= amount;
    pthread_mutex_unlock(&acc->mutex);
    return SUCCESS;
}

void print_error(int code) {
    switch (code) {
        case ERROR_ACCOUNT_CLOSED: printf("Error: Account is closed.\n"); break;
        case ERROR_INSUFFICIENT_FUNDS: printf("Error: Insufficient funds.\n"); break;
        case ERROR_INVALID_ARGUMENT: printf("Error: Invalid argument.\n"); break;
        default: printf("An unknown error occurred.\n"); break;
    }
}

// For concurrent test
void* worker(void* arg) {
    BankAccount* acc = (BankAccount*)arg;
    for (int i = 0; i < 1000; ++i) {
        deposit(acc, 10);
        withdraw(acc, 5);
    }
    return NULL;
}


void run_tests() {
    long long bal;
    int status;
    
    // Test Case 1: Basic Operations
    printf("--- Test Case 1: Basic Operations ---\n");
    BankAccount* account1 = openAccount(1000);
    if (!account1) { printf("Failed to open account.\n"); return; }
    getBalance(account1, &bal);
    printf("Initial Balance: %lld\n", bal);
    deposit(account1, 500);
    getBalance(account1, &bal);
    printf("Balance after deposit: %lld\n", bal);
    withdraw(account1, 200);
    getBalance(account1, &bal);
    printf("Balance after withdrawal: %lld\n", bal);

    // Test Case 2: Insufficient Funds
    printf("\n--- Test Case 2: Insufficient Funds ---\n");
    getBalance(account1, &bal);
    printf("Current Balance: %lld\n", bal);
    printf("Attempting to withdraw 2000...\n");
    status = withdraw(account1, 2000);
    if (status != SUCCESS) {
        print_error(status);
    }

    // Test Case 3: Operations on a closed account
    printf("\n--- Test Case 3: Operations on Closed Account ---\n");
    closeAccount(account1);
    printf("Account closed.\n");
    status = deposit(account1, 100);
    if (status != SUCCESS) {
        printf("Caught expected error on deposit. ");
        print_error(status);
    }
    status = getBalance(account1, &bal);
    if (status != SUCCESS) {
        printf("Caught expected error on getBalance. ");
        print_error(status);
    }
    destroyAccount(account1);

    // Test Case 4: Concurrent Transactions
    printf("\n--- Test Case 4: Concurrent Transactions ---\n");
    BankAccount* account2 = openAccount(10000);
    pthread_t threads[10];
    for (int i = 0; i < 10; ++i) {
        pthread_create(&threads[i], NULL, worker, account2);
    }
    for (int i = 0; i < 10; ++i) {
        pthread_join(threads[i], NULL);
    }
    printf("Initial concurrent balance: 10000\n");
    printf("After 10 threads each doing 1000 deposits of 10 and 1000 withdrawals of 5...\n");
    // Expected: 10000 + 10 * 1000 * (10 - 5) = 10000 + 50000 = 60000
    getBalance(account2, &bal);
    printf("Final concurrent balance: %lld\n", bal);
    destroyAccount(account2);

    // Test Case 5: Opening with negative balance
    printf("\n--- Test Case 5: Opening with Negative Balance ---\n");
    BankAccount* account3 = openAccount(-100);
    if (account3 == NULL) {
        printf("Caught expected error: Cannot open account with negative balance.\n");
    } else {
        destroyAccount(account3);
    }
}

int main() {
    run_tests();
    return 0;
}