
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#define MAX_ACCOUNTS 1000
#define ACCOUNT_ID_LEN 50

typedef struct {
    double balance;
    bool closed;
    pthread_rwlock_t lock;
} Account;

typedef struct {
    char accountId[ACCOUNT_ID_LEN];
    Account* account;
} AccountEntry;

typedef struct {
    AccountEntry entries[MAX_ACCOUNTS];
    int count;
    pthread_mutex_t mutex;
} Task181;

Task181* createTask181() {
    Task181* bank = (Task181*)malloc(sizeof(Task181));
    if (bank == NULL) {
        return NULL;
    }
    bank->count = 0;
    pthread_mutex_init(&bank->mutex, NULL);
    return bank;
}

Account* createAccount(double initialBalance) {
    Account* account = (Account*)malloc(sizeof(Account));
    if (account == NULL) {
        return NULL;
    }
    account->balance = initialBalance;
    account->closed = false;
    pthread_rwlock_init(&account->lock, NULL);
    return account;
}

bool openAccount(Task181* bank, const char* accountId, double initialBalance) {
    if (bank == NULL || accountId == NULL || initialBalance < 0) {
        return false;
    }
    
    pthread_mutex_lock(&bank->mutex);
    
    for (int i = 0; i < bank->count; i++) {
        if (strcmp(bank->entries[i].accountId, accountId) == 0) {
            pthread_mutex_unlock(&bank->mutex);
            return false;
        }
    }
    
    if (bank->count >= MAX_ACCOUNTS) {
        pthread_mutex_unlock(&bank->mutex);
        return false;
    }
    
    Account* account = createAccount(initialBalance);
    if (account == NULL) {
        pthread_mutex_unlock(&bank->mutex);
        return false;
    }
    
    strncpy(bank->entries[bank->count].accountId, accountId, ACCOUNT_ID_LEN - 1);
    bank->entries[bank->count].accountId[ACCOUNT_ID_LEN - 1] = '\\0';
    bank->entries[bank->count].account = account;
    bank->count++;
    
    pthread_mutex_unlock(&bank->mutex);
    return true;
}

Account* findAccount(Task181* bank, const char* accountId) {
    for (int i = 0; i < bank->count; i++) {
        if (strcmp(bank->entries[i].accountId, accountId) == 0) {
            return bank->entries[i].account;
        }
    }
    return NULL;
}

bool deposit(Task181* bank, const char* accountId, double amount) {
    if (bank == NULL || accountId == NULL || amount <= 0) {
        return false;
    }
    
    pthread_mutex_lock(&bank->mutex);
    Account* account = findAccount(bank, accountId);
    pthread_mutex_unlock(&bank->mutex);
    
    if (account == NULL) {
        return false;
    }
    
    pthread_rwlock_wrlock(&account->lock);
    if (account->closed) {
        pthread_rwlock_unlock(&account->lock);
        return false;
    }
    account->balance += amount;
    pthread_rwlock_unlock(&account->lock);
    return true;
}

bool withdraw(Task181* bank, const char* accountId, double amount) {
    if (bank == NULL || accountId == NULL || amount <= 0) {
        return false;
    }
    
    pthread_mutex_lock(&bank->mutex);
    Account* account = findAccount(bank, accountId);
    pthread_mutex_unlock(&bank->mutex);
    
    if (account == NULL) {
        return false;
    }
    
    pthread_rwlock_wrlock(&account->lock);
    if (account->closed) {
        pthread_rwlock_unlock(&account->lock);
        return false;
    }
    if (account->balance < amount) {
        pthread_rwlock_unlock(&account->lock);
        return false;
    }
    account->balance -= amount;
    pthread_rwlock_unlock(&account->lock);
    return true;
}

bool getBalance(Task181* bank, const char* accountId, double* outBalance) {
    if (bank == NULL || accountId == NULL || outBalance == NULL) {
        return false;
    }
    
    pthread_mutex_lock(&bank->mutex);
    Account* account = findAccount(bank, accountId);
    pthread_mutex_unlock(&bank->mutex);
    
    if (account == NULL) {
        return false;
    }
    
    pthread_rwlock_rdlock(&account->lock);
    if (account->closed) {
        pthread_rwlock_unlock(&account->lock);
        return false;
    }
    *outBalance = account->balance;
    pthread_rwlock_unlock(&account->lock);
    return true;
}

bool closeAccount(Task181* bank, const char* accountId) {
    if (bank == NULL || accountId == NULL) {
        return false;
    }
    
    pthread_mutex_lock(&bank->mutex);
    Account* account = findAccount(bank, accountId);
    pthread_mutex_unlock(&bank->mutex);
    
    if (account == NULL) {
        return false;
    }
    
    pthread_rwlock_wrlock(&account->lock);
    if (account->closed) {
        pthread_rwlock_unlock(&account->lock);
        return false;
    }
    account->closed = true;
    pthread_rwlock_unlock(&account->lock);
    return true;
}

void destroyTask181(Task181* bank) {
    if (bank == NULL) {
        return;
    }
    
    for (int i = 0; i < bank->count; i++) {
        if (bank->entries[i].account != NULL) {
            pthread_rwlock_destroy(&bank->entries[i].account->lock);
            free(bank->entries[i].account);
        }
    }
    pthread_mutex_destroy(&bank->mutex);
    free(bank);
}

int main() {
    Task181* bank = createTask181();
    
    // Test Case 1: Open account and check balance
    printf("Test 1: Open account\\n");
    bool opened = openAccount(bank, "ACC001", 1000.00);
    printf("Account opened: %s\\n", opened ? "true" : "false");
    double balance;
    if (getBalance(bank, "ACC001", &balance)) {
        printf("Balance: %.2f\\n", balance);
    }
    
    // Test Case 2: Deposit money
    printf("\\nTest 2: Deposit money\\n");
    bool deposited = deposit(bank, "ACC001", 500.00);
    printf("Deposit successful: %s\\n", deposited ? "true" : "false");
    if (getBalance(bank, "ACC001", &balance)) {
        printf("Balance: %.2f\\n", balance);
    }
    
    // Test Case 3: Withdraw money
    printf("\\nTest 3: Withdraw money\\n");
    bool withdrawn = withdraw(bank, "ACC001", 300.00);
    printf("Withdrawal successful: %s\\n", withdrawn ? "true" : "false");
    if (getBalance(bank, "ACC001", &balance)) {
        printf("Balance: %.2f\\n", balance);
    }
    
    // Test Case 4: Close account and try operations
    printf("\\nTest 4: Close account\\n");
    bool closed = closeAccount(bank, "ACC001");
    printf("Account closed: %s\\n", closed ? "true" : "false");
    bool depositAfterClose = deposit(bank, "ACC001", 100.00);
    printf("Deposit after close: %s\\n", depositAfterClose ? "true" : "false");
    bool hasBalance = getBalance(bank, "ACC001", &balance);
    printf("Balance after close: %s\\n", hasBalance ? "Available" : "Not available");
    
    // Test Case 5: Insufficient funds
    printf("\\nTest 5: Insufficient funds\\n");
    openAccount(bank, "ACC002", 100.00);
    bool withdrawMore = withdraw(bank, "ACC002", 200.00);
    printf("Withdrawal (insufficient funds): %s\\n", withdrawMore ? "true" : "false");
    if (getBalance(bank, "ACC002", &balance)) {
        printf("Balance: %.2f\\n", balance);
    }
    
    destroyTask181(bank);
    return 0;
}
