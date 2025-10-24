#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Error codes
#define SUCCESS 0
#define ERR_CLOSED -1
#define ERR_FUNDS -2
#define ERR_INVALID_AMOUNT -3
#define ERR_ALREADY_OPEN -4

typedef struct {
    long balance;
    int is_open;
    pthread_mutex_t lock;
} BankAccount;

// Function declarations
void bank_account_init(BankAccount* acc);
void bank_account_destroy(BankAccount* acc);
int bank_account_open(BankAccount* acc, long initial_deposit);
void bank_account_close(BankAccount* acc);
int bank_account_get_balance(BankAccount* acc, long* balance);
int bank_account_deposit(BankAccount* acc, long amount);
int bank_account_withdraw(BankAccount* acc, long amount);

// Function to initialize the bank account struct
void bank_account_init(BankAccount* acc) {
    acc->balance = 0;
    acc->is_open = 0;
    pthread_mutex_init(&acc->lock, NULL);
}

// Function to destroy the bank account struct (cleanup mutex)
void bank_account_destroy(BankAccount* acc) {
    pthread_mutex_destroy(&acc->lock);
}

int bank_account_open(BankAccount* acc, long initial_deposit) {
    pthread_mutex_lock(&acc->lock);
    if (initial_deposit < 0) {
        pthread_mutex_unlock(&acc->lock);
        return ERR_INVALID_AMOUNT;
    }
    acc->balance = initial_deposit;
    acc->is_open = 1;
    pthread_mutex_unlock(&acc->lock);
    return SUCCESS;
}

void bank_account_close(BankAccount* acc) {
    pthread_mutex_lock(&acc->lock);
    acc->is_open = 0;
    pthread_mutex_unlock(&acc->lock);
}

int bank_account_get_balance(BankAccount* acc, long* balance) {
    pthread_mutex_lock(&acc->lock);
    if (!acc->is_open) {
        pthread_mutex_unlock(&acc->lock);
        return ERR_CLOSED;
    }
    *balance = acc->balance;
    pthread_mutex_unlock(&acc->lock);
    return SUCCESS;
}

int bank_account_deposit(BankAccount* acc, long amount) {
    pthread_mutex_lock(&acc->lock);
    if (!acc->is_open) {
        pthread_mutex_unlock(&acc->lock);
        return ERR_CLOSED;
    }
    if (amount <= 0) {
        pthread_mutex_unlock(&acc->lock);
        return ERR_INVALID_AMOUNT;
    }
    acc->balance += amount;
    pthread_mutex_unlock(&acc->lock);
    return SUCCESS;
}

int bank_account_withdraw(BankAccount* acc, long amount) {
    pthread_mutex_lock(&acc->lock);
    if (!acc->is_open) {
        pthread_mutex_unlock(&acc->lock);
        return ERR_CLOSED;
    }
    if (amount <= 0) {
        pthread_mutex_unlock(&acc->lock);
        return ERR_INVALID_AMOUNT;
    }
    if (acc->balance < amount) {
        pthread_mutex_unlock(&acc->lock);
        return ERR_FUNDS;
    }
    acc->balance -= amount;
    pthread_mutex_unlock(&acc->lock);
    return SUCCESS;
}

typedef struct {
    BankAccount* account;
    int is_depositor;
    int transactions;
} WorkerArgs;

void* concurrent_worker(void* args) {
    WorkerArgs* w_args = (WorkerArgs*)args;
    for (int i = 0; i < w_args->transactions; ++i) {
        if (w_args->is_depositor) {
            bank_account_deposit(w_args->account, 10);
        } else {
            bank_account_withdraw(w_args->account, 10);
        }
    }
    return NULL;
}

const char* error_to_string(int code) {
    switch (code) {
        case SUCCESS: return "SUCCESS";
        case ERR_CLOSED: return "Account is closed.";
        case ERR_FUNDS: return "Insufficient funds.";
        case ERR_INVALID_AMOUNT: return "Invalid amount.";
        case ERR_ALREADY_OPEN: return "Account already open.";
        default: return "Unknown error.";
    }
}

int main() {
    printf("--- C Bank Account Tests ---\n");

    // Test Case 1: Simple Deposit/Withdraw
    printf("\n--- Test Case 1: Simple Transactions ---\n");
    BankAccount acc1;
    bank_account_init(&acc1);
    bank_account_open(&acc1, 100);
    long balance;
    bank_account_get_balance(&acc1, &balance);
    printf("Opened with balance: %ld\n", balance);
    bank_account_deposit(&acc1, 50);
    bank_account_get_balance(&acc1, &balance);
    printf("After deposit of 50, balance: %ld\n", balance);
    bank_account_withdraw(&acc1, 30);
    bank_account_get_balance(&acc1, &balance);
    printf("After withdrawal of 30, balance: %ld\n", balance);
    if (balance == 120) printf("Test Case 1 PASSED\n"); else printf("Test Case 1 FAILED\n");

    // Test Case 2: Insufficient Funds
    printf("\n--- Test Case 2: Insufficient Funds ---\n");
    printf("Attempting to withdraw 200 from %ld...\n", balance);
    int result = bank_account_withdraw(&acc1, 200);
    if (result == ERR_FUNDS) {
        printf("Caught expected error: %s\n", error_to_string(result));
        printf("Test Case 2 PASSED\n");
    } else {
        printf("Test Case 2 FAILED (Expected ERR_FUNDS, got %d)\n", result);
    }
    
    // Test Case 3: Closed Account Operations
    printf("\n--- Test Case 3: Closed Account ---\n");
    bank_account_close(&acc1);
    printf("Account closed.\n");
    result = bank_account_get_balance(&acc1, &balance);
    if(result == ERR_CLOSED) {
        printf("Caught expected error on getBalance: %s\n", error_to_string(result));
        printf("Test Case 3 PASSED\n");
    } else {
         printf("Test Case 3 FAILED (getBalance on closed account)\n");
    }
    bank_account_destroy(&acc1);

    // Test Case 4: Concurrent Transactions
    printf("\n--- Test Case 4: Concurrent Transactions ---\n");
    BankAccount shared_account;
    bank_account_init(&shared_account);
    bank_account_open(&shared_account, 10000);
    int transactions = 1000;
    int num_thread_pairs = 10;
    int total_threads = num_thread_pairs * 2;
    pthread_t threads[total_threads];
    WorkerArgs args[total_threads];
    for (int i = 0; i < num_thread_pairs; ++i) {
        int d_idx = i * 2;
        int w_idx = i * 2 + 1;
        args[d_idx] = (WorkerArgs){&shared_account, 1, transactions};
        pthread_create(&threads[d_idx], NULL, concurrent_worker, &args[d_idx]);
        args[w_idx] = (WorkerArgs){&shared_account, 0, transactions};
        pthread_create(&threads[w_idx], NULL, concurrent_worker, &args[w_idx]);
    }
    for (int i = 0; i < total_threads; ++i) pthread_join(threads[i], NULL);
    long final_balance;
    bank_account_get_balance(&shared_account, &final_balance);
    printf("Initial Balance: 10000\n");
    printf("Final Balance after concurrent transactions: %ld\n", final_balance);
    if (final_balance == 10000) printf("Test Case 4 PASSED\n"); else printf("Test Case 4 FAILED\n");
    bank_account_destroy(&shared_account);

    // Test Case 5: Zero/Negative Amount
    printf("\n--- Test Case 5: Invalid Amount ---\n");
    BankAccount acc5;
    bank_account_init(&acc5);
    bank_account_open(&acc5, 100);
    printf("Attempting to deposit -50...\n");
    result = bank_account_deposit(&acc5, -50);
    if (result == ERR_INVALID_AMOUNT) {
        printf("Caught expected error: %s\n", error_to_string(result));
    } else {
        printf("Test Case 5 FAILED (negative deposit)\n");
    }
    printf("Attempting to withdraw 0...\n");
    result = bank_account_withdraw(&acc5, 0);
    if (result == ERR_INVALID_AMOUNT) {
        printf("Caught expected error: %s\n", error_to_string(result));
        printf("Test Case 5 PASSED\n");
    } else {
        printf("Test Case 5 FAILED (zero withdrawal)\n");
    }
    bank_account_destroy(&acc5);

    return 0;
}