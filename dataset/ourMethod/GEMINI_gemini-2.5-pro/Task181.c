#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#define MAX_ACCOUNTS 100
#define ACCOUNT_NOT_FOUND -1
#define OPERATION_FAILED -2
#define INSUFFICIENT_FUNDS -3
#define ACCOUNT_CLOSED -4
#define INVALID_ARGUMENT -5
#define SUCCESS 0

typedef struct {
    long id;
    double balance;
    bool is_open;
    bool is_in_use;
    pthread_mutex_t lock;
} Account;

typedef struct {
    Account accounts[MAX_ACCOUNTS];
    pthread_mutex_t bank_lock; // For creating accounts
} Bank;

void bank_init(Bank* bank) {
    if (bank == NULL) return;
    pthread_mutex_init(&bank->bank_lock, NULL);
    for (int i = 0; i < MAX_ACCOUNTS; ++i) {
        bank->accounts[i].is_in_use = false;
        pthread_mutex_init(&bank->accounts[i].lock, NULL);
    }
}

void bank_destroy(Bank* bank) {
    if (bank == NULL) return;
    pthread_mutex_destroy(&bank->bank_lock);
    for (int i = 0; i < MAX_ACCOUNTS; ++i) {
        pthread_mutex_destroy(&bank->accounts[i].lock);
    }
}

long open_account(Bank* bank, double initial_balance) {
    if (bank == NULL || initial_balance < 0) {
        return INVALID_ARGUMENT;
    }
    pthread_mutex_lock(&bank->bank_lock);
    long account_id = -1;
    for (int i = 0; i < MAX_ACCOUNTS; ++i) {
        if (!bank->accounts[i].is_in_use) {
            account_id = i;
            bank->accounts[i].id = account_id;
            bank->accounts[i].balance = initial_balance;
            bank->accounts[i].is_open = true;
            bank->accounts[i].is_in_use = true;
            break;
        }
    }
    pthread_mutex_unlock(&bank->bank_lock);
    return account_id; // Returns -1 if no space
}

int close_account(Bank* bank, long account_id) {
    if (bank == NULL || account_id < 0 || account_id >= MAX_ACCOUNTS) {
        return ACCOUNT_NOT_FOUND;
    }
    pthread_mutex_lock(&bank->accounts[account_id].lock);
    if (!bank->accounts[account_id].is_in_use) {
        pthread_mutex_unlock(&bank->accounts[account_id].lock);
        return ACCOUNT_NOT_FOUND;
    }
    bank->accounts[account_id].is_open = false;
    pthread_mutex_unlock(&bank->accounts[account_id].lock);
    return SUCCESS;
}

int deposit(Bank* bank, long account_id, double amount) {
    if (bank == NULL || account_id < 0 || account_id >= MAX_ACCOUNTS) return ACCOUNT_NOT_FOUND;
    if (amount <= 0) return INVALID_ARGUMENT;

    pthread_mutex_lock(&bank->accounts[account_id].lock);
    if (!bank->accounts[account_id].is_in_use) {
        pthread_mutex_unlock(&bank->accounts[account_id].lock);
        return ACCOUNT_NOT_FOUND;
    }
    if (!bank->accounts[account_id].is_open) {
        pthread_mutex_unlock(&bank->accounts[account_id].lock);
        return ACCOUNT_CLOSED;
    }
    bank->accounts[account_id].balance += amount;
    pthread_mutex_unlock(&bank->accounts[account_id].lock);
    return SUCCESS;
}

int withdraw(Bank* bank, long account_id, double amount) {
    if (bank == NULL || account_id < 0 || account_id >= MAX_ACCOUNTS) return ACCOUNT_NOT_FOUND;
    if (amount <= 0) return INVALID_ARGUMENT;
    
    pthread_mutex_lock(&bank->accounts[account_id].lock);
    if (!bank->accounts[account_id].is_in_use) {
        pthread_mutex_unlock(&bank->accounts[account_id].lock);
        return ACCOUNT_NOT_FOUND;
    }
    if (!bank->accounts[account_id].is_open) {
        pthread_mutex_unlock(&bank->accounts[account_id].lock);
        return ACCOUNT_CLOSED;
    }
    if (bank->accounts[account_id].balance < amount) {
        pthread_mutex_unlock(&bank->accounts[account_id].lock);
        return INSUFFICIENT_FUNDS;
    }
    bank->accounts[account_id].balance -= amount;
    pthread_mutex_unlock(&bank->accounts[account_id].lock);
    return SUCCESS;
}

int get_balance(Bank* bank, long account_id, double* balance) {
    if (bank == NULL || account_id < 0 || account_id >= MAX_ACCOUNTS) return ACCOUNT_NOT_FOUND;
    
    pthread_mutex_lock(&bank->accounts[account_id].lock);
    if (!bank->accounts[account_id].is_in_use) {
        pthread_mutex_unlock(&bank->accounts[account_id].lock);
        return ACCOUNT_NOT_FOUND;
    }
    if (!bank->accounts[account_id].is_open) {
        pthread_mutex_unlock(&bank->accounts[account_id].lock);
        return ACCOUNT_CLOSED;
    }
    *balance = bank->accounts[account_id].balance;
    pthread_mutex_unlock(&bank->accounts[account_id].lock);
    return SUCCESS;
}

int main() {
    Bank bank;
    bank_init(&bank);
    double balance;

    printf("--- Bank System Test ---\n");

    // Test Case 1: Open an account and check initial balance
    printf("\n--- Test Case 1: Open Account ---\n");
    long acc1 = open_account(&bank, 100.0);
    if (get_balance(&bank, acc1, &balance) == SUCCESS) {
        printf("Opened account %ld with balance: %.2f\n", acc1, balance);
    }

    // Test Case 2: Deposit money
    printf("\n--- Test Case 2: Deposit ---\n");
    printf("Depositing 50.0 into account %ld\n", acc1);
    deposit(&bank, acc1, 50.0);
    if (get_balance(&bank, acc1, &balance) == SUCCESS) {
        printf("New balance for account %ld: %.2f\n", acc1, balance);
    }

    // Test Case 3: Withdraw money
    printf("\n--- Test Case 3: Withdraw ---\n");
    printf("Withdrawing 30.0 from account %ld\n", acc1);
    withdraw(&bank, acc1, 30.0);
    if (get_balance(&bank, acc1, &balance) == SUCCESS) {
        printf("New balance for account %ld: %.2f\n", acc1, balance);
    }

    // Test Case 4: Attempt to withdraw more than available funds
    printf("\n--- Test Case 4: Insufficient Funds ---\n");
    printf("Attempting to withdraw 200.0 from account %ld\n", acc1);
    int status = withdraw(&bank, acc1, 200.0);
    if (status == INSUFFICIENT_FUNDS) {
        printf("Caught expected error: Insufficient funds\n");
        if (get_balance(&bank, acc1, &balance) == SUCCESS) {
            printf("Balance remains: %.2f\n", balance);
        }
    }

    // Test Case 5: Close account and attempt a transaction
    printf("\n--- Test Case 5: Closed Account ---\n");
    printf("Closing account %ld\n", acc1);
    close_account(&bank, acc1);
    printf("Attempting to deposit 10.0 into closed account %ld\n", acc1);
    status = deposit(&bank, acc1, 10.0);
    if (status == ACCOUNT_CLOSED) {
        printf("Caught expected error: Account is closed\n");
    }
    printf("Attempting to get balance of closed account %ld\n", acc1);
    status = get_balance(&bank, acc1, &balance);
    if (status == ACCOUNT_CLOSED) {
        printf("Caught expected error: Account is closed\n");
    }

    bank_destroy(&bank);
    return 0;
}