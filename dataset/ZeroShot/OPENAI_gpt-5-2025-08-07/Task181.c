#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <limits.h>
#include <stdatomic.h>

typedef struct Account {
    long long id;
    pthread_mutex_t lock;
    int closed;
    int64_t balance;
    struct Account* next;
} Account;

typedef struct {
    Account* head;
    pthread_mutex_t list_lock;
    atomic_long next_id;
} Bank;

void bank_init(Bank* bank) {
    bank->head = NULL;
    pthread_mutex_init(&bank->list_lock, NULL);
    atomic_store(&bank->next_id, 1);
}

static Account* bank_find_account_unsafe(Bank* bank, long long id) {
    Account* cur = bank->head;
    while (cur) {
        if (cur->id == id) return cur;
        cur = cur->next;
    }
    return NULL;
}

long long open_account(Bank* bank, int64_t initial_cents) {
    if (initial_cents < 0) return -1;
    Account* acc = (Account*)calloc(1, sizeof(Account));
    if (!acc) return -1;
    acc->id = atomic_fetch_add(&bank->next_id, 1);
    pthread_mutex_init(&acc->lock, NULL);
    acc->closed = 0;
    acc->balance = initial_cents;
    acc->next = NULL;

    pthread_mutex_lock(&bank->list_lock);
    acc->next = bank->head;
    bank->head = acc;
    pthread_mutex_unlock(&bank->list_lock);
    return acc->id;
}

int close_account(Bank* bank, long long account_id) {
    pthread_mutex_lock(&bank->list_lock);
    Account* acc = bank_find_account_unsafe(bank, account_id);
    pthread_mutex_unlock(&bank->list_lock);
    if (!acc) return 0;
    pthread_mutex_lock(&acc->lock);
    if (acc->closed) {
        pthread_mutex_unlock(&acc->lock);
        return 0;
    }
    acc->closed = 1;
    pthread_mutex_unlock(&acc->lock);
    return 1;
}

int deposit(Bank* bank, long long account_id, int64_t cents) {
    if (cents <= 0) return 0;
    pthread_mutex_lock(&bank->list_lock);
    Account* acc = bank_find_account_unsafe(bank, account_id);
    pthread_mutex_unlock(&bank->list_lock);
    if (!acc) return 0;
    pthread_mutex_lock(&acc->lock);
    if (acc->closed) {
        pthread_mutex_unlock(&acc->lock);
        return 0;
    }
    if (acc->balance > LLONG_MAX - cents) {
        pthread_mutex_unlock(&acc->lock);
        return 0; // overflow
    }
    acc->balance += cents;
    pthread_mutex_unlock(&acc->lock);
    return 1;
}

int withdraw(Bank* bank, long long account_id, int64_t cents) {
    if (cents <= 0) return 0;
    pthread_mutex_lock(&bank->list_lock);
    Account* acc = bank_find_account_unsafe(bank, account_id);
    pthread_mutex_unlock(&bank->list_lock);
    if (!acc) return 0;
    pthread_mutex_lock(&acc->lock);
    if (acc->closed) {
        pthread_mutex_unlock(&acc->lock);
        return 0;
    }
    if (cents > acc->balance) {
        pthread_mutex_unlock(&acc->lock);
        return 0; // insufficient
    }
    acc->balance -= cents;
    pthread_mutex_unlock(&acc->lock);
    return 1;
}

// returns success via return value, and writes balance to out if success
int get_balance(Bank* bank, long long account_id, int64_t* out_balance) {
    if (!out_balance) return 0;
    pthread_mutex_lock(&bank->list_lock);
    Account* acc = bank_find_account_unsafe(bank, account_id);
    pthread_mutex_unlock(&bank->list_lock);
    if (!acc) return 0;
    pthread_mutex_lock(&acc->lock);
    if (acc->closed) {
        pthread_mutex_unlock(&acc->lock);
        return 0;
    }
    *out_balance = acc->balance;
    pthread_mutex_unlock(&acc->lock);
    return 1;
}

typedef struct {
    Bank* bank;
    long long account_id;
    int iterations;
    int64_t amount;
    atomic_int* start_flag;
} worker_args;

void* deposit_worker(void* arg) {
    worker_args* wa = (worker_args*)arg;
    while (atomic_load(wa->start_flag) == 0) { /* spin */ }
    for (int i = 0; i < wa->iterations; ++i) {
        deposit(wa->bank, wa->account_id, wa->amount);
    }
    return NULL;
}

void* mixed_worker(void* arg) {
    worker_args* wa = (worker_args*)arg;
    while (atomic_load(wa->start_flag) == 0) { /* spin */ }
    for (int i = 0; i < wa->iterations; ++i) {
        deposit(wa->bank, wa->account_id, wa->amount);
        withdraw(wa->bank, wa->account_id, wa->amount);
    }
    return NULL;
}

int main() {
    Bank bank;
    bank_init(&bank);

    // Test 1
    long long a1 = open_account(&bank, 10000);
    int d1 = deposit(&bank, a1, 5000);
    int w1 = withdraw(&bank, a1, 2000);
    int64_t b1 = 0;
    int g1 = get_balance(&bank, a1, &b1);
    printf("Test1: %d\n", (d1 && w1 && g1 && b1 == 13000));

    // Test 2: Concurrent deposits
    long long a2 = open_account(&bank, 0);
    int threads2 = 10;
    int perOps2 = 1000;
    int64_t perOp2 = 1000;
    pthread_t ts2[10];
    worker_args args2[10];
    atomic_int start2;
    atomic_store(&start2, 0);
    for (int i = 0; i < threads2; ++i) {
        args2[i].bank = &bank;
        args2[i].account_id = a2;
        args2[i].iterations = perOps2;
        args2[i].amount = perOp2;
        args2[i].start_flag = &start2;
        pthread_create(&ts2[i], NULL, deposit_worker, &args2[i]);
    }
    atomic_store(&start2, 1);
    for (int i = 0; i < threads2; ++i) pthread_join(ts2[i], NULL);
    int64_t b2 = 0;
    int g2 = get_balance(&bank, a2, &b2);
    long long expected2 = 1LL * threads2 * perOps2 * perOp2;
    printf("Test2: %d\n", (g2 && b2 == expected2));

    // Test 3: Mixed operations
    long long a3 = open_account(&bank, 1000000);
    int threads3 = 8;
    int perOps3 = 5000;
    pthread_t ts3[8];
    worker_args args3[8];
    atomic_int start3;
    atomic_store(&start3, 0);
    for (int i = 0; i < threads3; ++i) {
        args3[i].bank = &bank;
        args3[i].account_id = a3;
        args3[i].iterations = perOps3;
        args3[i].amount = 100;
        args3[i].start_flag = &start3;
        pthread_create(&ts3[i], NULL, mixed_worker, &args3[i]);
    }
    atomic_store(&start3, 1);
    for (int i = 0; i < threads3; ++i) pthread_join(ts3[i], NULL);
    int64_t b3 = 0;
    int g3 = get_balance(&bank, a3, &b3);
    printf("Test3: %d\n", (g3 && b3 == 1000000));

    // Test 4: Close and fail ops
    long long a4 = open_account(&bank, 50000);
    int c4 = close_account(&bank, a4);
    int d4 = deposit(&bank, a4, 1000);
    int w4 = withdraw(&bank, a4, 1000);
    int64_t b4 = 0;
    int g4 = get_balance(&bank, a4, &b4);
    printf("Test4: %d\n", (c4 && !d4 && !w4 && !g4));

    // Test 5: Insufficient and invalid
    long long a5 = open_account(&bank, 5000);
    int w5a = withdraw(&bank, a5, 10000);
    int d5a = deposit(&bank, a5, -100);
    int w5b = withdraw(&bank, a5, 0);
    int64_t b5 = 0;
    int g5 = get_balance(&bank, a5, &b5);
    printf("Test5: %d\n", (!w5a && !d5a && !w5b && g5 && b5 == 5000));

    return 0;
}