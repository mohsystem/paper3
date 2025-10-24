#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

typedef struct {
    pthread_mutex_t mutex;
    int open;
    long long balance;
} BankAccount;

void bank_account_init(BankAccount* acc) {
    pthread_mutex_init(&acc->mutex, NULL);
    acc->open = 0;
    acc->balance = 0;
}

int bank_account_open(BankAccount* acc) {
    pthread_mutex_lock(&acc->mutex);
    if (acc->open) { pthread_mutex_unlock(&acc->mutex); return -1; }
    acc->open = 1;
    acc->balance = 0;
    pthread_mutex_unlock(&acc->mutex);
    return 0;
}

int bank_account_close(BankAccount* acc) {
    pthread_mutex_lock(&acc->mutex);
    if (!acc->open) { pthread_mutex_unlock(&acc->mutex); return -1; }
    if (acc->balance != 0) { pthread_mutex_unlock(&acc->mutex); return -2; }
    acc->open = 0;
    pthread_mutex_unlock(&acc->mutex);
    return 0;
}

int bank_account_deposit(BankAccount* acc, long long amount, long long* new_balance) {
    if (amount < 0) return -3;
    pthread_mutex_lock(&acc->mutex);
    if (!acc->open) { pthread_mutex_unlock(&acc->mutex); return -1; }
    acc->balance += amount;
    if (new_balance) *new_balance = acc->balance;
    pthread_mutex_unlock(&acc->mutex);
    return 0;
}

int bank_account_withdraw(BankAccount* acc, long long amount, long long* new_balance) {
    if (amount < 0) return -3;
    pthread_mutex_lock(&acc->mutex);
    if (!acc->open) { pthread_mutex_unlock(&acc->mutex); return -1; }
    if (amount > acc->balance) { pthread_mutex_unlock(&acc->mutex); return -4; }
    acc->balance -= amount;
    if (new_balance) *new_balance = acc->balance;
    pthread_mutex_unlock(&acc->mutex);
    return 0;
}

int bank_account_get_balance(BankAccount* acc, long long* out_balance) {
    pthread_mutex_lock(&acc->mutex);
    if (!acc->open) { pthread_mutex_unlock(&acc->mutex); return -1; }
    if (out_balance) *out_balance = acc->balance;
    pthread_mutex_unlock(&acc->mutex);
    return 0;
}

typedef struct {
    BankAccount* acc;
    int times;
    long long amount;
    int is_deposit; // 1 deposit, 0 withdraw
} ThreadOpArgs;

void* thread_op(void* arg) {
    ThreadOpArgs* args = (ThreadOpArgs*)arg;
    for (int i = 0; i < args->times; ++i) {
        if (args->is_deposit) {
            bank_account_deposit(args->acc, args->amount, NULL);
        } else {
            // ignore errors (like insufficient funds) for this stress test
            bank_account_withdraw(args->acc, args->amount, NULL);
        }
    }
    return NULL;
}

int main() {
    // Test 1: Simple open, deposit, withdraw
    BankAccount a1; bank_account_init(&a1); bank_account_open(&a1);
    bank_account_deposit(&a1, 200, NULL);
    bank_account_withdraw(&a1, 50, NULL);
    long long bal = 0;
    bank_account_get_balance(&a1, &bal);
    printf("Test1 balance (expected 150): %lld\n", bal);

    // Test 2: Concurrent deposits
    BankAccount a2; bank_account_init(&a2); bank_account_open(&a2);
    int threads2 = 8;
    int depositsPerThread2 = 10000;
    pthread_t th2[8];
    ThreadOpArgs args2 = { .acc=&a2, .times=depositsPerThread2, .amount=1, .is_deposit=1 };
    for (int i = 0; i < threads2; ++i) {
        pthread_create(&th2[i], NULL, thread_op, &args2);
    }
    for (int i = 0; i < threads2; ++i) pthread_join(th2[i], NULL);
    bank_account_get_balance(&a2, &bal);
    printf("Test2 balance (expected %d): %lld\n", threads2 * depositsPerThread2, bal);

    // Test 3: Over-withdraw should fail gracefully
    BankAccount a3; bank_account_init(&a3); bank_account_open(&a3);
    bank_account_deposit(&a3, 50, NULL);
    int rc = bank_account_withdraw(&a3, 100, NULL);
    if (rc == 0) {
        printf("Test3 unexpected success\n");
    } else {
        printf("Test3 over-withdraw failed as expected: err=%d\n", rc);
    }

    // Test 4: Close then attempt operation should fail
    BankAccount a4; bank_account_init(&a4); bank_account_open(&a4);
    bank_account_deposit(&a4, 75, NULL);
    bank_account_withdraw(&a4, 75, NULL);
    rc = bank_account_close(&a4);
    if (rc != 0) printf("Test4 close failed err=%d\n", rc);
    rc = bank_account_deposit(&a4, 10, NULL);
    if (rc == 0) {
        printf("Test4 unexpected success\n");
    } else {
        printf("Test4 operation on closed account failed as expected: err=%d\n", rc);
    }

    // Test 5: Mixed concurrent operations
    BankAccount a5; bank_account_init(&a5); bank_account_open(&a5);
    bank_account_deposit(&a5, 1000, NULL);
    int withdrawThreads = 5, withdrawsPerThread = 100; // total 500
    int depositThreads = 3, depositsPerThread = 100;   // total 300
    int totalThreads = withdrawThreads + depositThreads;
    pthread_t th5[16];
    ThreadOpArgs withdrawArgs = { .acc=&a5, .times=withdrawsPerThread, .amount=1, .is_deposit=0 };
    ThreadOpArgs depositArgs  = { .acc=&a5, .times=depositsPerThread, .amount=1, .is_deposit=1 };
    int idx = 0;
    for (int i = 0; i < withdrawThreads; ++i) pthread_create(&th5[idx++], NULL, thread_op, &withdrawArgs);
    for (int i = 0; i < depositThreads; ++i) pthread_create(&th5[idx++], NULL, thread_op, &depositArgs);
    for (int i = 0; i < totalThreads; ++i) pthread_join(th5[i], NULL);
    bank_account_get_balance(&a5, &bal);
    printf("Test5 balance (expected 800): %lld\n", bal);

    return 0;
}