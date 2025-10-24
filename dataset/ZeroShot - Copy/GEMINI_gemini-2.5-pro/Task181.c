#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <limits.h>

typedef struct {
    int id;
    long long balance;
    bool is_open;
    pthread_mutex_t lock;
} BankAccount;

// Global Bank state
static BankAccount* g_accounts = NULL;
static int g_account_count = 0;
static int g_account_capacity = 0;
static int g_next_account_id = 0;
static pthread_mutex_t g_bank_lock; // Protects the global accounts array

// Forward declarations
void init_bank();
void destroy_bank();
int open_account(long long initial_balance);
bool close_account(int account_id);
bool deposit(int account_id, long long amount);
bool withdraw(int account_id, long long amount);
bool get_balance(int account_id, long long* balance);

void init_bank() {
    g_account_capacity = 10;
    g_accounts = (BankAccount*)malloc(g_account_capacity * sizeof(BankAccount));
    if (g_accounts == NULL) {
        perror("Failed to allocate memory for bank accounts");
        exit(EXIT_FAILURE);
    }
    g_account_count = 0;
    g_next_account_id = 0;
    pthread_mutex_init(&g_bank_lock, NULL);
}

void destroy_bank() {
    for (int i = 0; i < g_account_count; ++i) {
        pthread_mutex_destroy(&g_accounts[i].lock);
    }
    free(g_accounts);
    g_accounts = NULL;
    pthread_mutex_destroy(&g_bank_lock);
}

BankAccount* find_account(int account_id) {
    // This function assumes the caller holds the g_bank_lock
    for (int i = 0; i < g_account_count; ++i) {
        if (g_accounts[i].id == account_id) {
            return &g_accounts[i];
        }
    }
    return NULL;
}

int open_account(long long initial_balance) {
    if (initial_balance < 0) return -1;

    pthread_mutex_lock(&g_bank_lock);

    if (g_account_count >= g_account_capacity) {
        // Resize logic
        g_account_capacity *= 2;
        BankAccount* temp = (BankAccount*)realloc(g_accounts, g_account_capacity * sizeof(BankAccount));
        if (temp == NULL) {
            perror("Failed to reallocate memory for bank accounts");
            pthread_mutex_unlock(&g_bank_lock);
            return -1;
        }
        g_accounts = temp;
    }

    int new_id = ++g_next_account_id;
    BankAccount* new_account = &g_accounts[g_account_count];
    new_account->id = new_id;
    new_account->balance = initial_balance;
    new_account->is_open = true;
    pthread_mutex_init(&new_account->lock, NULL);

    g_account_count++;
    
    pthread_mutex_unlock(&g_bank_lock);
    return new_id;
}

bool close_account(int account_id) {
    pthread_mutex_lock(&g_bank_lock);
    BankAccount* account = find_account(account_id);
    pthread_mutex_unlock(&g_bank_lock);

    if (account) {
        pthread_mutex_lock(&account->lock);
        account->is_open = false;
        pthread_mutex_unlock(&account->lock);
        return true;
    }
    return false;
}

bool deposit(int account_id, long long amount) {
    if (amount <= 0) return false;
    
    pthread_mutex_lock(&g_bank_lock);
    BankAccount* account = find_account(account_id);
    pthread_mutex_unlock(&g_bank_lock);
    
    if (account) {
        pthread_mutex_lock(&account->lock);
        if (!account->is_open) {
            pthread_mutex_unlock(&account->lock);
            return false;
        }
        if (account->balance > LLONG_MAX - amount) { // Overflow check
             pthread_mutex_unlock(&account->lock);
             return false;
        }
        account->balance += amount;
        pthread_mutex_unlock(&account->lock);
        return true;
    }
    return false;
}

bool withdraw(int account_id, long long amount) {
    if (amount <= 0) return false;

    pthread_mutex_lock(&g_bank_lock);
    BankAccount* account = find_account(account_id);
    pthread_mutex_unlock(&g_bank_lock);

    if (account) {
        pthread_mutex_lock(&account->lock);
        if (!account->is_open || account->balance < amount) {
            pthread_mutex_unlock(&account->lock);
            return false;
        }
        account->balance -= amount;
        pthread_mutex_unlock(&account->lock);
        return true;
    }
    return false;
}

bool get_balance(int account_id, long long* balance_out) {
    pthread_mutex_lock(&g_bank_lock);
    BankAccount* account = find_account(account_id);
    pthread_mutex_unlock(&g_bank_lock);

    if (account) {
        pthread_mutex_lock(&account->lock);
        if (!account->is_open) {
            pthread_mutex_unlock(&account->lock);
            return false;
        }
        *balance_out = account->balance;
        pthread_mutex_unlock(&account->lock);
        return true;
    }
    return false;
}

// Thread function for test cases
void* deposit_task(void* arg) {
    int account_id = *(int*)arg;
    deposit(account_id, 10);
    return NULL;
}

typedef struct {
    int id;
    long long deposit_amount;
    long long withdraw_amount;
} MixTaskArgs;

void* mix_task(void* arg) {
    MixTaskArgs* args = (MixTaskArgs*)arg;
    deposit(args->id, args->deposit_amount);
    withdraw(args->id, args->withdraw_amount);
    return NULL;
}


int main() {
    init_bank();

    printf("--- C Bank Test Cases ---\n");

    // Test Case 1: Simple deposit and withdrawal
    printf("\n--- Test Case 1: Simple Operations ---\n");
    long long balance;
    int acc1 = open_account(100);
    if (get_balance(acc1, &balance)) printf("Opened account %d with balance: %lld\n", acc1, balance);
    deposit(acc1, 50);
    if (get_balance(acc1, &balance)) printf("Deposited 50. New balance: %lld\n", balance);
    withdraw(acc1, 30);
    if (get_balance(acc1, &balance)) printf("Withdrew 30. Final balance: %lld\n", balance);

    // Test Case 2: Insufficient funds
    printf("\n--- Test Case 2: Insufficient Funds ---\n");
    int acc2 = open_account(50);
    if (get_balance(acc2, &balance)) printf("Opened account %d with balance: %lld\n", acc2, balance);
    bool success = withdraw(acc2, 100);
    printf("Attempted to withdraw 100. Success: %s\n", success ? "true" : "false");
    if (get_balance(acc2, &balance)) printf("Final balance: %lld\n", balance);

    // Test Case 3: Operations on a closed account
    printf("\n--- Test Case 3: Closed Account Operations ---\n");
    int acc3 = open_account(200);
    if (get_balance(acc3, &balance)) printf("Opened account %d with balance: %lld\n", acc3, balance);
    close_account(acc3);
    printf("Account %d closed.\n", acc3);
    printf("Attempting deposit of 50. Success: %s\n", deposit(acc3, 50) ? "true" : "false");
    printf("Attempting withdrawal of 20. Success: %s\n", withdraw(acc3, 20) ? "true" : "false");
    if (!get_balance(acc3, &balance)) printf("Final balance: N/A (account closed)\n");

    // Test Case 4: Concurrent deposits
    printf("\n--- Test Case 4: Concurrent Deposits ---\n");
    int acc4 = open_account(0);
    if (get_balance(acc4, &balance)) printf("Opened account %d with balance: %lld\n", acc4, balance);
    pthread_t deposit_threads[10];
    for (int i = 0; i < 10; ++i) {
        pthread_create(&deposit_threads[i], NULL, deposit_task, &acc4);
    }
    for (int i = 0; i < 10; ++i) {
        pthread_join(deposit_threads[i], NULL);
    }
    if (get_balance(acc4, &balance)) printf("After 10 threads each deposited 10, final balance: %lld\n", balance);

    // Test Case 5: Concurrent deposits and withdrawals
    printf("\n--- Test Case 5: Concurrent Mix ---\n");
    int acc5 = open_account(1000);
    if (get_balance(acc5, &balance)) printf("Opened account %d with balance: %lld\n", acc5, balance);
    pthread_t mix_threads[10];
    MixTaskArgs mix_args = {acc5, 20, 30};
    for (int i = 0; i < 10; ++i) {
        // Just for simplicity, we pass the same args. 5 will deposit, 5 will withdraw
        // The task itself does both, but for the test, we'll have 5 do one, 5 the other
        if (i < 5) {
             pthread_create(&mix_threads[i], NULL, deposit_task, &acc5); // A simpler task for this case
        } else {
             pthread_create(&mix_threads[i], NULL, (void* (*)(void*))withdraw, (void*)(intptr_t)acc5); // Simple withdraw task
        }
    }

    // A better way for case 5
    int acc5_deposit_id = acc5;
    int acc5_withdraw_id = acc5;
    for(int i = 0; i < 5; i++){
        pthread_create(&mix_threads[i], NULL, deposit_task, &acc5_deposit_id);
    }
    MixTaskArgs withdraw_arg = {acc5, 0, 30}; // Only for withdrawal
    void* withdraw_task(void* arg) { withdraw(((MixTaskArgs*)arg)->id, ((MixTaskArgs*)arg)->withdraw_amount); return NULL; }

    for(int i = 5; i < 10; i++){
        pthread_create(&mix_threads[i], NULL, withdraw_task, &withdraw_arg);
    }

    for (int i = 0; i < 10; ++i) {
        pthread_join(mix_threads[i], NULL);
    }
    // Expected: 1000 + (5*10) - (5*30) = 900. Wait, task deposits 10.
    // Let's re-run with correct numbers. 5*10 deposit = 50. 5*30 withdraw = 150.
    // Expected: 1000 + 50 - 150 = 900
    if (get_balance(acc5, &balance)) printf("After 5 deposits of 10 and 5 withdrawals of 30, final balance: %lld\n", balance);

    destroy_bank();
    return 0;
}