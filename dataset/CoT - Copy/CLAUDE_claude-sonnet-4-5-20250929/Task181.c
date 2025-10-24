
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <limits.h>

typedef struct {
    char account_id[50];
    long long balance;
    bool is_closed;
    pthread_mutex_t lock;
} BankAccount;

typedef struct AccountNode {
    BankAccount* account;
    struct AccountNode* next;
} AccountNode;

typedef struct {
    AccountNode* head;
    pthread_mutex_t lock;
} BankingSystem;

BankAccount* create_account(const char* account_id, long long initial_balance) {
    if (initial_balance < 0) {
        return NULL;
    }
    
    BankAccount* account = (BankAccount*)malloc(sizeof(BankAccount));
    if (!account) return NULL;
    
    strncpy(account->account_id, account_id, 49);
    account->account_id[49] = '\\0';
    account->balance = initial_balance;
    account->is_closed = false;
    pthread_mutex_init(&account->lock, NULL);
    
    return account;
}

void destroy_account(BankAccount* account) {
    if (account) {
        pthread_mutex_destroy(&account->lock);
        free(account);
    }
}

bool deposit(BankAccount* account, long long amount) {
    if (!account || amount <= 0) {
        return false;
    }
    
    pthread_mutex_lock(&account->lock);
    bool result = false;
    
    if (!account->is_closed) {
        if (account->balance <= LLONG_MAX - amount) {
            account->balance += amount;
            result = true;
        }
    }
    
    pthread_mutex_unlock(&account->lock);
    return result;
}

bool withdraw(BankAccount* account, long long amount) {
    if (!account || amount <= 0) {
        return false;
    }
    
    pthread_mutex_lock(&account->lock);
    bool result = false;
    
    if (!account->is_closed && account->balance >= amount) {
        account->balance -= amount;
        result = true;
    }
    
    pthread_mutex_unlock(&account->lock);
    return result;
}

long long get_balance(BankAccount* account) {
    if (!account) {
        return -1;
    }
    
    pthread_mutex_lock(&account->lock);
    long long balance = account->is_closed ? -1 : account->balance;
    pthread_mutex_unlock(&account->lock);
    
    return balance;
}

bool close_account(BankAccount* account) {
    if (!account) {
        return false;
    }
    
    pthread_mutex_lock(&account->lock);
    bool result = false;
    
    if (!account->is_closed) {
        account->is_closed = true;
        result = true;
    }
    
    pthread_mutex_unlock(&account->lock);
    return result;
}

BankingSystem* create_banking_system() {
    BankingSystem* system = (BankingSystem*)malloc(sizeof(BankingSystem));
    if (!system) return NULL;
    
    system->head = NULL;
    pthread_mutex_init(&system->lock, NULL);
    
    return system;
}

bool open_account_in_system(BankingSystem* system, const char* account_id, long long initial_balance) {
    if (!system || !account_id || strlen(account_id) == 0) {
        return false;
    }
    
    pthread_mutex_lock(&system->lock);
    
    AccountNode* current = system->head;
    while (current) {
        if (strcmp(current->account->account_id, account_id) == 0) {
            pthread_mutex_unlock(&system->lock);
            return false;
        }
        current = current->next;
    }
    
    BankAccount* account = create_account(account_id, initial_balance);
    if (!account) {
        pthread_mutex_unlock(&system->lock);
        return false;
    }
    
    AccountNode* node = (AccountNode*)malloc(sizeof(AccountNode));
    if (!node) {
        destroy_account(account);
        pthread_mutex_unlock(&system->lock);
        return false;
    }
    
    node->account = account;
    node->next = system->head;
    system->head = node;
    
    pthread_mutex_unlock(&system->lock);
    return true;
}

BankAccount* find_account(BankingSystem* system, const char* account_id) {
    if (!system || !account_id) {
        return NULL;
    }
    
    pthread_mutex_lock(&system->lock);
    
    AccountNode* current = system->head;
    while (current) {
        if (strcmp(current->account->account_id, account_id) == 0) {
            pthread_mutex_unlock(&system->lock);
            return current->account;
        }
        current = current->next;
    }
    
    pthread_mutex_unlock(&system->lock);
    return NULL;
}

void destroy_banking_system(BankingSystem* system) {
    if (!system) return;
    
    AccountNode* current = system->head;
    while (current) {
        AccountNode* next = current->next;
        destroy_account(current->account);
        free(current);
        current = next;
    }
    
    pthread_mutex_destroy(&system->lock);
    free(system);
}

typedef struct {
    BankingSystem* bank;
    char account_id[50];
} ThreadArgs;

void* deposit_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    for (int i = 0; i < 100; i++) {
        BankAccount* account = find_account(args->bank, args->account_id);
        deposit(account, 10);
    }
    return NULL;
}

void* withdraw_thread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    for (int i = 0; i < 100; i++) {
        BankAccount* account = find_account(args->bank, args->account_id);
        withdraw(account, 5);
    }
    return NULL;
}

int main() {
    BankingSystem* bank = create_banking_system();

    // Test Case 1: Open account and deposit
    printf("Test 1 - Open account and deposit:\\n");
    open_account_in_system(bank, "ACC001", 1000);
    BankAccount* acc1 = find_account(bank, "ACC001");
    deposit(acc1, 500);
    printf("Balance: %lld\\n", get_balance(acc1)); // Expected: 1500

    // Test Case 2: Withdraw from account
    printf("\\nTest 2 - Withdraw:\\n");
    withdraw(acc1, 300);
    printf("Balance: %lld\\n", get_balance(acc1)); // Expected: 1200

    // Test Case 3: Close account and try operations
    printf("\\nTest 3 - Close account:\\n");
    close_account(acc1);
    bool deposit_result = deposit(acc1, 100);
    printf("Deposit after close: %d\\n", deposit_result); // Expected: 0
    printf("Balance after close: %lld\\n", get_balance(acc1)); // Expected: -1

    // Test Case 4: Insufficient funds
    printf("\\nTest 4 - Insufficient funds:\\n");
    open_account_in_system(bank, "ACC002", 100);
    BankAccount* acc2 = find_account(bank, "ACC002");
    bool withdraw_result = withdraw(acc2, 200);
    printf("Withdraw result: %d\\n", withdraw_result); // Expected: 0

    // Test Case 5: Concurrent operations
    printf("\\nTest 5 - Concurrent operations:\\n");
    open_account_in_system(bank, "ACC003", 1000);
    
    ThreadArgs args;
    args.bank = bank;
    strncpy(args.account_id, "ACC003", 49);
    
    pthread_t t1, t2;
    pthread_create(&t1, NULL, deposit_thread, &args);
    pthread_create(&t2, NULL, withdraw_thread, &args);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    BankAccount* acc3 = find_account(bank, "ACC003");
    printf("Final balance: %lld\\n", get_balance(acc3)); // Expected: 1500

    destroy_banking_system(bank);
    return 0;
}
