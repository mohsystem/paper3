
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#define MAX_ACCOUNTS 1000
#define MAX_ACCOUNT_ID 50

typedef struct {
    char accountId[MAX_ACCOUNT_ID];
    double balance;
    bool closed;
    bool active;
    pthread_mutex_t lock;
} BankAccount;

typedef struct {
    BankAccount accounts[MAX_ACCOUNTS];
    int count;
    pthread_mutex_t globalLock;
} Task181;

Task181* createBankingSystem() {
    Task181* bank = (Task181*)malloc(sizeof(Task181));
    bank->count = 0;
    pthread_mutex_init(&bank->globalLock, NULL);
    for (int i = 0; i < MAX_ACCOUNTS; i++) {
        bank->accounts[i].active = false;
        pthread_mutex_init(&bank->accounts[i].lock, NULL);
    }
    return bank;
}

void destroyBankingSystem(Task181* bank) {
    pthread_mutex_destroy(&bank->globalLock);
    for (int i = 0; i < MAX_ACCOUNTS; i++) {
        pthread_mutex_destroy(&bank->accounts[i].lock);
    }
    free(bank);
}

BankAccount* findAccount(Task181* bank, const char* accountId) {
    for (int i = 0; i < MAX_ACCOUNTS; i++) {
        if (bank->accounts[i].active && 
            strcmp(bank->accounts[i].accountId, accountId) == 0) {
            return &bank->accounts[i];
        }
    }
    return NULL;
}

bool openAccount(Task181* bank, const char* accountId, double initialBalance) {
    if (accountId == NULL || initialBalance < 0) {
        return false;
    }

    pthread_mutex_lock(&bank->globalLock);
    
    if (findAccount(bank, accountId) != NULL) {
        pthread_mutex_unlock(&bank->globalLock);
        return false;
    }

    for (int i = 0; i < MAX_ACCOUNTS; i++) {
        if (!bank->accounts[i].active) {
            strncpy(bank->accounts[i].accountId, accountId, MAX_ACCOUNT_ID - 1);
            bank->accounts[i].accountId[MAX_ACCOUNT_ID - 1] = '\\0';
            bank->accounts[i].balance = initialBalance;
            bank->accounts[i].closed = false;
            bank->accounts[i].active = true;
            bank->count++;
            pthread_mutex_unlock(&bank->globalLock);
            return true;
        }
    }
    
    pthread_mutex_unlock(&bank->globalLock);
    return false;
}

bool closeAccount(Task181* bank, const char* accountId) {
    pthread_mutex_lock(&bank->globalLock);
    BankAccount* account = findAccount(bank, accountId);
    pthread_mutex_unlock(&bank->globalLock);
    
    if (account == NULL) {
        return false;
    }

    pthread_mutex_lock(&account->lock);
    if (account->closed) {
        pthread_mutex_unlock(&account->lock);
        return false;
    }
    account->closed = true;
    pthread_mutex_unlock(&account->lock);
    return true;
}

bool deposit(Task181* bank, const char* accountId, double amount) {
    pthread_mutex_lock(&bank->globalLock);
    BankAccount* account = findAccount(bank, accountId);
    pthread_mutex_unlock(&bank->globalLock);
    
    if (account == NULL) {
        return false;
    }

    pthread_mutex_lock(&account->lock);
    if (account->closed || amount <= 0) {
        pthread_mutex_unlock(&account->lock);
        return false;
    }
    account->balance += amount;
    pthread_mutex_unlock(&account->lock);
    return true;
}

bool withdraw(Task181* bank, const char* accountId, double amount) {
    pthread_mutex_lock(&bank->globalLock);
    BankAccount* account = findAccount(bank, accountId);
    pthread_mutex_unlock(&bank->globalLock);
    
    if (account == NULL) {
        return false;
    }

    pthread_mutex_lock(&account->lock);
    if (account->closed || amount <= 0 || amount > account->balance) {
        pthread_mutex_unlock(&account->lock);
        return false;
    }
    account->balance -= amount;
    pthread_mutex_unlock(&account->lock);
    return true;
}

double getBalance(Task181* bank, const char* accountId) {
    pthread_mutex_lock(&bank->globalLock);
    BankAccount* account = findAccount(bank, accountId);
    pthread_mutex_unlock(&bank->globalLock);
    
    if (account == NULL) {
        return -1;
    }

    pthread_mutex_lock(&account->lock);
    if (account->closed) {
        pthread_mutex_unlock(&account->lock);
        return -1;
    }
    double balance = account->balance;
    pthread_mutex_unlock(&account->lock);
    return balance;
}

typedef struct {
    Task181* bank;
    char accountId[MAX_ACCOUNT_ID];
} ThreadData;

void* depositThread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    for (int i = 0; i < 100; i++) {
        deposit(data->bank, data->accountId, 10.0);
    }
    return NULL;
}

void* withdrawThread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    for (int i = 0; i < 100; i++) {
        withdraw(data->bank, data->accountId, 5.0);
    }
    return NULL;
}

int main() {
    Task181* bank = createBankingSystem();

    // Test Case 1: Open account and deposit
    printf("Test Case 1: Open account and deposit\\n");
    openAccount(bank, "ACC001", 1000.0);
    deposit(bank, "ACC001", 500.0);
    printf("Balance: %.2f\\n", getBalance(bank, "ACC001"));

    // Test Case 2: Withdraw from account
    printf("\\nTest Case 2: Withdraw from account\\n");
    withdraw(bank, "ACC001", 300.0);
    printf("Balance: %.2f\\n", getBalance(bank, "ACC001"));

    // Test Case 3: Close account and try operations
    printf("\\nTest Case 3: Close account and try operations\\n");
    closeAccount(bank, "ACC001");
    printf("Deposit after close: %d\\n", deposit(bank, "ACC001", 100.0));
    printf("Balance after close: %.2f\\n", getBalance(bank, "ACC001"));

    // Test Case 4: Concurrent operations
    printf("\\nTest Case 4: Concurrent operations\\n");
    openAccount(bank, "ACC002", 5000.0);
    
    pthread_t t1, t2;
    ThreadData data;
    data.bank = bank;
    strncpy(data.accountId, "ACC002", MAX_ACCOUNT_ID);
    
    pthread_create(&t1, NULL, depositThread, &data);
    pthread_create(&t2, NULL, withdrawThread, &data);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Balance after concurrent ops: %.2f\\n", getBalance(bank, "ACC002"));

    // Test Case 5: Invalid operations
    printf("\\nTest Case 5: Invalid operations\\n");
    printf("Open duplicate account: %d\\n", openAccount(bank, "ACC002", 1000.0));
    printf("Withdraw from non-existent: %d\\n", withdraw(bank, "ACC999", 100.0));
    printf("Overdraw: %d\\n", withdraw(bank, "ACC002", 10000.0));

    destroyBankingSystem(bank);
    return 0;
}
