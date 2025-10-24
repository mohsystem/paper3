/* Chain-of-Through process in code comments:
1) Problem understanding: Thread-safe bank for open/close/deposit/withdraw/get_balance.
2) Security requirements: Validate inputs, avoid overflows, handle closed/non-existent accounts gracefully.
3) Secure coding generation: pthread mutex per account; bank map protected by mutex; do not free accounts after creation to keep pointers valid.
4) Code review: Check all error paths; avoid deadlocks by locking at most one account mutex at a time.
5) Secure code output: Final C implementation with 5 test cases.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

typedef struct Account {
    long long id;
    long long balance;
    int open; // 1 open, 0 closed
    pthread_mutex_t mtx;
    struct Account* next;
} Account;

typedef struct Bank {
    Account* head;
    pthread_mutex_t map_mtx;
    long long nextId;
} Bank;

// Initialize bank
void bank_init(Bank* bank) {
    bank->head = NULL;
    bank->nextId = 1000;
    pthread_mutex_init(&bank->map_mtx, NULL);
}

// Helper: find account by id (no locking on account here)
Account* bank_get_account_unsafe(Bank* bank, long long id) {
    Account* cur = bank->head;
    while (cur) {
        if (cur->id == id) return cur;
        cur = cur->next;
    }
    return NULL;
}

// Open account
long long bank_open_account(Bank* bank, long long initialCents) {
    if (initialCents < 0) return -1;
    Account* acc = (Account*)calloc(1, sizeof(Account));
    if (!acc) return -1;
    pthread_mutex_init(&acc->mtx, NULL);
    acc->balance = initialCents;
    acc->open = 1;
    acc->next = NULL;
    pthread_mutex_lock(&bank->map_mtx);
    acc->id = bank->nextId++;
    acc->next = bank->head;
    bank->head = acc;
    pthread_mutex_unlock(&bank->map_mtx);
    return acc->id;
}

// Close account (only if balance is zero)
int bank_close_account(Bank* bank, long long id) {
    pthread_mutex_lock(&bank->map_mtx);
    Account* acc = bank_get_account_unsafe(bank, id);
    pthread_mutex_unlock(&bank->map_mtx);
    if (!acc) return 0;
    pthread_mutex_lock(&acc->mtx);
    int ok = 0;
    if (acc->open && acc->balance == 0) {
        acc->open = 0;
        ok = 1;
    }
    pthread_mutex_unlock(&acc->mtx);
    return ok;
}

// Deposit
int bank_deposit(Bank* bank, long long id, long long amountCents) {
    if (amountCents <= 0) return 0;
    pthread_mutex_lock(&bank->map_mtx);
    Account* acc = bank_get_account_unsafe(bank, id);
    pthread_mutex_unlock(&bank->map_mtx);
    if (!acc) return 0;
    pthread_mutex_lock(&acc->mtx);
    int ok = 0;
    if (acc->open) {
        if (acc->balance <= LLONG_MAX - amountCents) {
            acc->balance += amountCents;
            ok = 1;
        }
    }
    pthread_mutex_unlock(&acc->mtx);
    return ok;
}

// Withdraw
int bank_withdraw(Bank* bank, long long id, long long amountCents) {
    if (amountCents <= 0) return 0;
    pthread_mutex_lock(&bank->map_mtx);
    Account* acc = bank_get_account_unsafe(bank, id);
    pthread_mutex_unlock(&bank->map_mtx);
    if (!acc) return 0;
    pthread_mutex_lock(&acc->mtx);
    int ok = 0;
    if (acc->open && acc->balance >= amountCents) {
        acc->balance -= amountCents;
        ok = 1;
    }
    pthread_mutex_unlock(&acc->mtx);
    return ok;
}

// Get balance (-1 if not found or closed)
long long bank_get_balance(Bank* bank, long long id) {
    pthread_mutex_lock(&bank->map_mtx);
    Account* acc = bank_get_account_unsafe(bank, id);
    pthread_mutex_unlock(&bank->map_mtx);
    if (!acc) return -1;
    pthread_mutex_lock(&acc->mtx);
    long long res = acc->open ? acc->balance : -1;
    pthread_mutex_unlock(&acc->mtx);
    return res;
}

// Thread args for concurrency tests
typedef struct {
    Bank* bank;
    long long id;
    int iters;
    long long amount;
    int is_deposit; // 1 deposit, 0 withdraw
} WorkerArgs;

void* worker_run(void* arg) {
    WorkerArgs* wa = (WorkerArgs*)arg;
    for (int i = 0; i < wa->iters; ++i) {
        if (wa->is_deposit) bank_deposit(wa->bank, wa->id, wa->amount);
        else bank_withdraw(wa->bank, wa->id, wa->amount);
    }
    return NULL;
}

// 5 test cases in main
int main() {
    Bank bank;
    bank_init(&bank);

    // Test 1: Basic operations
    long long acc1 = bank_open_account(&bank, 10000);
    int d1 = bank_deposit(&bank, acc1, 5000);
    int w1 = bank_withdraw(&bank, acc1, 3000);
    long long b1 = bank_get_balance(&bank, acc1);
    printf("Test1 balance expected 12000: %lld ok=%d\n", b1, (d1 && w1 && b1 == 12000));

    // Test 2: Concurrent deposits
    long long acc2 = bank_open_account(&bank, 0);
    int threads2 = 10, iters2 = 100;
    long long amount2 = 1000;
    pthread_t th2[10];
    WorkerArgs args2[10];
    for (int i = 0; i < threads2; ++i) {
        args2[i].bank = &bank;
        args2[i].id = acc2;
        args2[i].iters = iters2;
        args2[i].amount = amount2;
        args2[i].is_deposit = 1;
        pthread_create(&th2[i], NULL, worker_run, &args2[i]);
    }
    for (int i = 0; i < threads2; ++i) pthread_join(th2[i], NULL);
    long long expected2 = (long long)threads2 * iters2 * amount2;
    long long b2 = bank_get_balance(&bank, acc2);
    printf("Test2 balance expected %lld: %lld ok=%d\n", expected2, b2, (b2 == expected2));

    // Test 3: Mixed concurrent ops
    long long acc3 = bank_open_account(&bank, 500000);
    int depThreads = 4, wthThreads = 3;
    int depIters = 100, wthIters = 80;
    long long depAmt = 1000, wthAmt = 1000;
    pthread_t th3[7];
    WorkerArgs args3[7];
    for (int i = 0; i < depThreads; ++i) {
        args3[i].bank = &bank;
        args3[i].id = acc3;
        args3[i].iters = depIters;
        args3[i].amount = depAmt;
        args3[i].is_deposit = 1;
        pthread_create(&th3[i], NULL, worker_run, &args3[i]);
    }
    for (int i = 0; i < wthThreads; ++i) {
        args3[depThreads + i].bank = &bank;
        args3[depThreads + i].id = acc3;
        args3[depThreads + i].iters = wthIters;
        args3[depThreads + i].amount = wthAmt;
        args3[depThreads + i].is_deposit = 0;
        pthread_create(&th3[depThreads + i], NULL, worker_run, &args3[depThreads + i]);
    }
    for (int i = 0; i < depThreads + wthThreads; ++i) pthread_join(th3[i], NULL);
    long long expected3 = 500000 + depThreads * 1LL * depIters * depAmt - wthThreads * 1LL * wthIters * wthAmt;
    long long b3 = bank_get_balance(&bank, acc3);
    printf("Test3 balance expected %lld: %lld ok=%d\n", expected3, b3, (b3 == expected3));

    // Test 4: Close account and operations must fail
    long long acc4 = bank_open_account(&bank, 0);
    int close4 = bank_close_account(&bank, acc4);
    int dep4 = bank_deposit(&bank, acc4, 1000);
    int wth4 = bank_withdraw(&bank, acc4, 1000);
    long long b4 = bank_get_balance(&bank, acc4);
    printf("Test4 close ok=%d depositFail=%d withdrawFail=%d balanceIsNeg1=%d\n",
           close4, !dep4, !wth4, (b4 == -1));

    // Test 5: Non-existent account
    long long fake = 9999999;
    int dep5 = bank_deposit(&bank, fake, 100);
    int wth5 = bank_withdraw(&bank, fake, 100);
    int close5 = bank_close_account(&bank, fake);
    long long b5 = bank_get_balance(&bank, fake);
    printf("Test5 all fail: %d\n", (!dep5 && !wth5 && !close5 && b5 == -1));

    return 0;
}