#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>
#include <ctype.h>

#define MAX_ACCOUNTS 128
#define ID_MAX 32

// Error codes
#define ERR_NO_ACCOUNT_OR_CLOSED (-1LL)
#define ERR_INSUFFICIENT_FUNDS (-2LL)
#define ERR_INVALID_INPUT (-3LL)

typedef struct {
    pthread_mutex_t mtx;
    int used;
    int closed;
    char id[ID_MAX + 1];
    long long balance;
} Account;

typedef struct {
    pthread_mutex_t map_mtx;
    Account accounts[MAX_ACCOUNTS];
} Bank;

static int valid_id(const char* id) {
    if (id == NULL) return 0;
    size_t len = strnlen(id, ID_MAX + 1);
    if (len == 0 || len > ID_MAX) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)id[i];
        if (!(isalnum(c) || c == '_' || c == '-')) return 0;
    }
    return 1;
}

static void bank_init(Bank* bank) {
    if (!bank) return;
    pthread_mutex_init(&bank->map_mtx, NULL);
    for (int i = 0; i < MAX_ACCOUNTS; i++) {
        pthread_mutex_init(&bank->accounts[i].mtx, NULL);
        bank->accounts[i].used = 0;
        bank->accounts[i].closed = 0;
        bank->accounts[i].id[0] = '\0';
        bank->accounts[i].balance = 0;
    }
}

static void bank_destroy(Bank* bank) {
    if (!bank) return;
    for (int i = 0; i < MAX_ACCOUNTS; i++) {
        pthread_mutex_destroy(&bank->accounts[i].mtx);
    }
    pthread_mutex_destroy(&bank->map_mtx);
}

static int find_account_index_unlocked(Bank* bank, const char* id) {
    for (int i = 0; i < MAX_ACCOUNTS; i++) {
        if (bank->accounts[i].used && strncmp(bank->accounts[i].id, id, ID_MAX) == 0) {
            return i;
        }
    }
    return -1;
}

int bank_open(Bank* bank, const char* id, long long initial_cents) {
    if (!bank || !valid_id(id) || initial_cents < 0) return 0;
    if (pthread_mutex_lock(&bank->map_mtx) != 0) return 0;
    int idx = find_account_index_unlocked(bank, id);
    if (idx >= 0) {
        pthread_mutex_unlock(&bank->map_mtx);
        return 0;
    }
    int free_idx = -1;
    for (int i = 0; i < MAX_ACCOUNTS; i++) {
        if (!bank->accounts[i].used) {
            free_idx = i;
            break;
        }
    }
    if (free_idx < 0) {
        pthread_mutex_unlock(&bank->map_mtx);
        return 0;
    }
    Account* acc = &bank->accounts[free_idx];
    acc->used = 1;
    acc->closed = 0;
    acc->balance = initial_cents;
    snprintf(acc->id, sizeof(acc->id), "%s", id);
    pthread_mutex_unlock(&bank->map_mtx);
    return 1;
}

long long bank_get_balance(Bank* bank, const char* id) {
    if (!bank || !valid_id(id)) return ERR_INVALID_INPUT;
    if (pthread_mutex_lock(&bank->map_mtx) != 0) return ERR_NO_ACCOUNT_OR_CLOSED;
    int idx = find_account_index_unlocked(bank, id);
    if (idx < 0) {
        pthread_mutex_unlock(&bank->map_mtx);
        return ERR_NO_ACCOUNT_OR_CLOSED;
    }
    Account* acc = &bank->accounts[idx];
    // Hold account lock for check-use atomicity
    pthread_mutex_lock(&acc->mtx);
    pthread_mutex_unlock(&bank->map_mtx);
    long long result;
    if (acc->closed) {
        result = ERR_NO_ACCOUNT_OR_CLOSED;
    } else {
        result = acc->balance;
    }
    pthread_mutex_unlock(&acc->mtx);
    return result;
}

long long bank_deposit(Bank* bank, const char* id, long long amount_cents) {
    if (!bank || !valid_id(id) || amount_cents < 0) return ERR_INVALID_INPUT;
    if (pthread_mutex_lock(&bank->map_mtx) != 0) return ERR_NO_ACCOUNT_OR_CLOSED;
    int idx = find_account_index_unlocked(bank, id);
    if (idx < 0) {
        pthread_mutex_unlock(&bank->map_mtx);
        return ERR_NO_ACCOUNT_OR_CLOSED;
    }
    Account* acc = &bank->accounts[idx];
    pthread_mutex_lock(&acc->mtx);
    pthread_mutex_unlock(&bank->map_mtx);
    long long result;
    if (acc->closed) {
        result = ERR_NO_ACCOUNT_OR_CLOSED;
    } else {
        long long newBal = acc->balance + amount_cents;
        if (newBal < 0) {
            result = ERR_INVALID_INPUT;
        } else {
            acc->balance = newBal;
            result = acc->balance;
        }
    }
    pthread_mutex_unlock(&acc->mtx);
    return result;
}

long long bank_withdraw(Bank* bank, const char* id, long long amount_cents) {
    if (!bank || !valid_id(id) || amount_cents < 0) return ERR_INVALID_INPUT;
    if (pthread_mutex_lock(&bank->map_mtx) != 0) return ERR_NO_ACCOUNT_OR_CLOSED;
    int idx = find_account_index_unlocked(bank, id);
    if (idx < 0) {
        pthread_mutex_unlock(&bank->map_mtx);
        return ERR_NO_ACCOUNT_OR_CLOSED;
    }
    Account* acc = &bank->accounts[idx];
    pthread_mutex_lock(&acc->mtx);
    pthread_mutex_unlock(&bank->map_mtx);
    long long result;
    if (acc->closed) {
        result = ERR_NO_ACCOUNT_OR_CLOSED;
    } else if (acc->balance < amount_cents) {
        result = ERR_INSUFFICIENT_FUNDS;
    } else {
        acc->balance -= amount_cents;
        result = acc->balance;
    }
    pthread_mutex_unlock(&acc->mtx);
    return result;
}

int bank_close(Bank* bank, const char* id) {
    if (!bank || !valid_id(id)) return 0;
    if (pthread_mutex_lock(&bank->map_mtx) != 0) return 0;
    int idx = find_account_index_unlocked(bank, id);
    if (idx < 0) {
        pthread_mutex_unlock(&bank->map_mtx);
        return 0;
    }
    Account* acc = &bank->accounts[idx];
    pthread_mutex_lock(&acc->mtx);
    pthread_mutex_unlock(&bank->map_mtx);
    int result = 0;
    if (!acc->closed && acc->balance == 0) {
        acc->closed = 1;
        result = 1;
    }
    pthread_mutex_unlock(&acc->mtx);
    return result;
}

typedef struct {
    Bank* bank;
    const char* id;
    int loops;
    long long amount;
    atomic_int* start_flag;
    int is_withdraw;
} ThreadArgs;

void* deposit_worker(void* arg) {
    ThreadArgs* a = (ThreadArgs*)arg;
    while (atomic_load(a->start_flag) == 0) {
        sched_yield();
    }
    for (int i = 0; i < a->loops; i++) {
        (void)bank_deposit(a->bank, a->id, a->amount);
    }
    return NULL;
}

void* withdraw_worker(void* arg) {
    ThreadArgs* a = (ThreadArgs*)arg;
    while (atomic_load(a->start_flag) == 0) {
        sched_yield();
    }
    for (int i = 0; i < a->loops; i++) {
        (void)bank_withdraw(a->bank, a->id, a->amount);
    }
    return NULL;
}

int main(void) {
    Bank bank;
    bank_init(&bank);

    // Test 1: Open and balance check
    int opened1 = bank_open(&bank, "alice", 10000);
    long long bal1 = bank_get_balance(&bank, "alice");
    printf("Test1: opened=%d balance=%lld\n", opened1, bal1);

    // Test 2: Deposit
    long long bal2 = bank_deposit(&bank, "alice", 2500);
    printf("Test2: deposit newBalance=%lld\n", bal2);

    // Test 3: Withdraw success
    long long bal3 = bank_withdraw(&bank, "alice", 5000);
    printf("Test3: withdraw newBalance=%lld\n", bal3);

    // Test 4: Withdraw insufficient
    long long w4 = bank_withdraw(&bank, "alice", 10000);
    long long bal4 = bank_get_balance(&bank, "alice");
    printf("Test4: withdrawResult=%lld balanceAfter=%lld\n", w4, bal4);

    // Test 5: Concurrency + close behavior
    const char* accId = "bob";
    int opened2 = bank_open(&bank, accId, 0);
    int depositThreads = 4;
    int depositIters = 1000;
    long long depositAmt = 100;
    int withdrawThreads = 2;
    int withdrawIters = 500;
    long long withdrawAmt = 50;

    pthread_t threads[6];
    ThreadArgs args[6];
    atomic_int start_flag;
    atomic_init(&start_flag, 0);

    for (int i = 0; i < depositThreads; i++) {
        args[i].bank = &bank;
        args[i].id = accId;
        args[i].loops = depositIters;
        args[i].amount = depositAmt;
        args[i].start_flag = &start_flag;
        args[i].is_withdraw = 0;
        pthread_create(&threads[i], NULL, deposit_worker, &args[i]);
    }
    for (int i = 0; i < withdrawThreads; i++) {
        int idx = depositThreads + i;
        args[idx].bank = &bank;
        args[idx].id = accId;
        args[idx].loops = withdrawIters;
        args[idx].amount = withdrawAmt;
        args[idx].start_flag = &start_flag;
        args[idx].is_withdraw = 1;
        pthread_create(&threads[idx], NULL, withdraw_worker, &args[idx]);
    }

    atomic_store(&start_flag, 1);
    for (int i = 0; i < depositThreads + withdrawThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    long long finalExpected = (long long)depositThreads * depositIters * depositAmt
                            - (long long)withdrawThreads * withdrawIters * withdrawAmt;
    long long bal5 = bank_get_balance(&bank, accId);
    int closeNonZero = bank_close(&bank, accId);
    long long withdrawAllRes = (bal5 >= 0) ? bank_withdraw(&bank, accId, bal5) : bal5;
    int closeZero = bank_close(&bank, accId);
    long long postCloseDeposit = bank_deposit(&bank, accId, 100);

    printf("Test5: opened=%d expectedBalance=%lld actualBalance=%lld closeNonZero=%d withdrawAllRes=%lld closeZero=%d depositAfterCloseRes=%lld\n",
           opened2, finalExpected, bal5, closeNonZero, withdrawAllRes, closeZero, postCloseDeposit);

    bank_destroy(&bank);
    return 0;
}