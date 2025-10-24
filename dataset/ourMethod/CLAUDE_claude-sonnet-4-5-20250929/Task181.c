
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

// Maximum number of accounts supported
#define MAX_ACCOUNTS 1000
#define ACCOUNT_ID_LEN 20

// Thread-safe bank account structure
typedef struct {
    char accountId[ACCOUNT_ID_LEN];
    double balance;
    bool closed;
    bool active;  // Indicates if slot is in use
    pthread_mutex_t mutex;  // Protects balance and closed state
} BankAccount;

// Bank system structure managing multiple accounts
typedef struct {
    BankAccount accounts[MAX_ACCOUNTS];
    pthread_mutex_t systemMutex;  // Protects accounts array access
    int nextAccountNum;
} BankSystem;

// Initialize a bank account - returns true on success
bool initBankAccount(BankAccount* account, const char* id, double initialBalance) {
    if (account == NULL || id == NULL || initialBalance < 0.0) {
        return false;
    }
    
    // Validate ID length to prevent buffer overflow
    if (strlen(id) >= ACCOUNT_ID_LEN) {
        return false;
    }
    
    strncpy(account->accountId, id, ACCOUNT_ID_LEN - 1);
    account->accountId[ACCOUNT_ID_LEN - 1] = '\\0';  // Ensure null termination
    account->balance = initialBalance;
    account->closed = false;
    account->active = true;
    
    // Initialize mutex with error checking
    if (pthread_mutex_init(&account->mutex, NULL) != 0) {
        return false;
    }
    
    return true;
}

// Clean up account resources
void destroyBankAccount(BankAccount* account) {
    if (account != NULL && account->active) {
        pthread_mutex_destroy(&account->mutex);
        account->active = false;
    }
}

// Initialize bank system
bool initBankSystem(BankSystem* bank) {
    if (bank == NULL) {
        return false;
    }
    
    // Initialize all accounts as inactive
    for (int i = 0; i < MAX_ACCOUNTS; i++) {
        bank->accounts[i].active = false;
    }
    
    bank->nextAccountNum = 1000;
    
    // Initialize system mutex with error checking
    if (pthread_mutex_init(&bank->systemMutex, NULL) != 0) {
        return false;
    }
    
    return true;
}

// Clean up bank system resources
void destroyBankSystem(BankSystem* bank) {
    if (bank == NULL) {
        return;
    }
    
    // Destroy all active account mutexes
    for (int i = 0; i < MAX_ACCOUNTS; i++) {
        if (bank->accounts[i].active) {
            destroyBankAccount(&bank->accounts[i]);
        }
    }
    
    pthread_mutex_destroy(&bank->systemMutex);
}

// Open new account - returns account ID or empty string on failure
bool openAccount(BankSystem* bank, double initialBalance, char* accountIdOut, size_t outSize) {
    if (bank == NULL || accountIdOut == NULL || outSize < ACCOUNT_ID_LEN || initialBalance < 0.0) {
        if (accountIdOut != NULL && outSize > 0) {
            accountIdOut[0] = '\\0';
        }
        return false;
    }
    
    pthread_mutex_lock(&bank->systemMutex);
    
    // Find empty slot
    int slot = -1;
    for (int i = 0; i < MAX_ACCOUNTS; i++) {
        if (!bank->accounts[i].active) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        pthread_mutex_unlock(&bank->systemMutex);
        accountIdOut[0] = '\\0';
        return false;  // No available slots
    }
    
    // Generate unique account ID
    char accountId[ACCOUNT_ID_LEN];
    int written = snprintf(accountId, ACCOUNT_ID_LEN, "ACC%d", bank->nextAccountNum);
    if (written < 0 || written >= ACCOUNT_ID_LEN) {
        pthread_mutex_unlock(&bank->systemMutex);
        accountIdOut[0] = '\\0';
        return false;
    }
    
    bank->nextAccountNum++;
    
    // Initialize account
    if (!initBankAccount(&bank->accounts[slot], accountId, initialBalance)) {
        pthread_mutex_unlock(&bank->systemMutex);
        accountIdOut[0] = '\\0';
        return false;
    }
    
    strncpy(accountIdOut, accountId, outSize - 1);
    accountIdOut[outSize - 1] = '\\0';
    
    pthread_mutex_unlock(&bank->systemMutex);
    return true;
}

// Find account by ID - caller must hold systemMutex
BankAccount* findAccount(BankSystem* bank, const char* accountId) {
    if (bank == NULL || accountId == NULL) {
        return NULL;
    }
    
    for (int i = 0; i < MAX_ACCOUNTS; i++) {
        if (bank->accounts[i].active && 
            strcmp(bank->accounts[i].accountId, accountId) == 0) {
            return &bank->accounts[i];
        }
    }
    
    return NULL;
}

// Deposit money to account
bool deposit(BankSystem* bank, const char* accountId, double amount) {
    if (bank == NULL || accountId == NULL || amount <= 0.0) {
        return false;
    }
    
    pthread_mutex_lock(&bank->systemMutex);
    BankAccount* account = findAccount(bank, accountId);
    pthread_mutex_unlock(&bank->systemMutex);
    
    if (account == NULL) {
        return false;  // Account not found
    }
    
    pthread_mutex_lock(&account->mutex);
    
    // Check if account is closed - fail gracefully
    if (account->closed) {
        pthread_mutex_unlock(&account->mutex);
        return false;
    }
    
    // Check for overflow
    if (account->balance + amount < account->balance) {
        pthread_mutex_unlock(&account->mutex);
        return false;
    }
    
    account->balance += amount;
    
    pthread_mutex_unlock(&account->mutex);
    return true;
}

// Withdraw money from account
bool withdraw(BankSystem* bank, const char* accountId, double amount) {
    if (bank == NULL || accountId == NULL || amount <= 0.0) {
        return false;
    }
    
    pthread_mutex_lock(&bank->systemMutex);
    BankAccount* account = findAccount(bank, accountId);
    pthread_mutex_unlock(&bank->systemMutex);
    
    if (account == NULL) {
        return false;  // Account not found
    }
    
    pthread_mutex_lock(&account->mutex);
    
    // Check if account is closed - fail gracefully
    if (account->closed) {
        pthread_mutex_unlock(&account->mutex);
        return false;
    }
    
    // Check sufficient funds atomically with withdrawal
    if (account->balance < amount) {
        pthread_mutex_unlock(&account->mutex);
        return false;
    }
    
    account->balance -= amount;
    
    pthread_mutex_unlock(&account->mutex);
    return true;
}

// Get account balance
bool getBalance(BankSystem* bank, const char* accountId, double* balance) {
    if (bank == NULL || accountId == NULL || balance == NULL) {
        return false;
    }
    
    pthread_mutex_lock(&bank->systemMutex);
    BankAccount* account = findAccount(bank, accountId);
    pthread_mutex_unlock(&bank->systemMutex);
    
    if (account == NULL) {
        return false;  // Account not found
    }
    
    pthread_mutex_lock(&account->mutex);
    *balance = account->balance;
    pthread_mutex_unlock(&account->mutex);
    
    return true;
}

// Close account
bool closeAccount(BankSystem* bank, const char* accountId) {
    if (bank == NULL || accountId == NULL) {
        return false;
    }
    
    pthread_mutex_lock(&bank->systemMutex);
    BankAccount* account = findAccount(bank, accountId);
    pthread_mutex_unlock(&bank->systemMutex);
    
    if (account == NULL) {
        return false;  // Account not found
    }
    
    pthread_mutex_lock(&account->mutex);
    
    if (account->closed) {
        pthread_mutex_unlock(&account->mutex);
        return false;  // Already closed
    }
    
    account->closed = true;
    
    pthread_mutex_unlock(&account->mutex);
    return true;
}

int main(void) {
    BankSystem bank;
    if (!initBankSystem(&bank)) {
        fprintf(stderr, "Failed to initialize bank system\\n");
        return 1;
    }
    
    char acc1[ACCOUNT_ID_LEN] = {0};
    char acc2[ACCOUNT_ID_LEN] = {0};
    char acc3[ACCOUNT_ID_LEN] = {0};
    double balance = 0.0;
    
    printf("=== Test Case 1: Open Account and Deposit ===\\n");
    if (openAccount(&bank, 1000.0, acc1, sizeof(acc1))) {
        printf("Opened account: %s\\n", acc1);
        if (getBalance(&bank, acc1, &balance)) {
            printf("Initial balance: $%.2f\\n", balance);
        }
        bool depositResult = deposit(&bank, acc1, 500.0);
        printf("Deposit $500: %s\\n", depositResult ? "Success" : "Failed");
        if (getBalance(&bank, acc1, &balance)) {
            printf("New balance: $%.2f\\n", balance);
        }
    }
    printf("\\n");
    
    printf("=== Test Case 2: Withdraw from Account ===\\n");
    bool withdrawResult = withdraw(&bank, acc1, 300.0);
    printf("Withdraw $300: %s\\n", withdrawResult ? "Success" : "Failed");
    if (getBalance(&bank, acc1, &balance)) {
        printf("Balance after withdrawal: $%.2f\\n", balance);
    }
    printf("\\n");
    
    printf("=== Test Case 3: Insufficient Funds ===\\n");
    withdrawResult = withdraw(&bank, acc1, 2000.0);
    printf("Withdraw $2000 (insufficient): %s\\n", withdrawResult ? "Success" : "Failed");
    if (getBalance(&bank, acc1, &balance)) {
        printf("Balance unchanged: $%.2f\\n", balance);
    }
    printf("\\n");
    
    printf("=== Test Case 4: Close Account ===\\n");
    bool closeResult = closeAccount(&bank, acc1);
    printf("Close account: %s\\n", closeResult ? "Success" : "Failed");
    bool depositResult = deposit(&bank, acc1, 100.0);
    printf("Deposit to closed account: %s\\n", depositResult ? "Success" : "Failed");
    printf("\\n");
    
    printf("=== Test Case 5: Multiple Accounts ===\\n");
    if (openAccount(&bank, 2000.0, acc2, sizeof(acc2))) {
        if (getBalance(&bank, acc2, &balance)) {
            printf("Opened account: %s with balance: $%.2f\\n", acc2, balance);
        }
    }
    if (openAccount(&bank, 3000.0, acc3, sizeof(acc3))) {
        if (getBalance(&bank, acc3, &balance)) {
            printf("Opened account: %s with balance: $%.2f\\n", acc3, balance);
        }
    }
    withdraw(&bank, acc2, 500.0);
    deposit(&bank, acc3, 1000.0);
    printf("After operations:\\n");
    if (getBalance(&bank, acc2, &balance)) {
        printf("%s balance: $%.2f\\n", acc2, balance);
    }
    if (getBalance(&bank, acc3, &balance)) {
        printf("%s balance: $%.2f\\n", acc3, balance);
    }
    
    destroyBankSystem(&bank);
    return 0;
}
