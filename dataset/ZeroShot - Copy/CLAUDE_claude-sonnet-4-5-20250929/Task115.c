
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

struct Task115 {
    uint8_t flag1 : 1;
    uint8_t flag2 : 1;
    uint8_t value : 4;
    uint8_t status : 2;
};

void Task115_init(struct Task115 *bf) {
    bf->flag1 = 0;
    bf->flag2 = 0;
    bf->value = 0;
    bf->status = 0;
}

void Task115_setFlag1(struct Task115 *bf, bool val) {
    bf->flag1 = val ? 1 : 0;
}

bool Task115_getFlag1(const struct Task115 *bf) {
    return bf->flag1 != 0;
}

void Task115_setFlag2(struct Task115 *bf, bool val) {
    bf->flag2 = val ? 1 : 0;
}

bool Task115_getFlag2(const struct Task115 *bf) {
    return bf->flag2 != 0;
}

void Task115_setValue(struct Task115 *bf, int val) {
    if (val < 0 || val > 15) {
        fprintf(stderr, "Error: Value must be between 0 and 15\\n");
        return;
    }
    bf->value = val;
}

int Task115_getValue(const struct Task115 *bf) {
    return bf->value;
}

void Task115_setStatus(struct Task115 *bf, int stat) {
    if (stat < 0 || stat > 3) {
        fprintf(stderr, "Error: Status must be between 0 and 3\\n");
        return;
    }
    bf->status = stat;
}

int Task115_getStatus(const struct Task115 *bf) {
    return bf->status;
}

void Task115_print(const struct Task115 *bf) {
    printf("BitField{flag1=%d, flag2=%d, value=%d, status=%d}\\n",
           Task115_getFlag1(bf),
           Task115_getFlag2(bf),
           Task115_getValue(bf),
           Task115_getStatus(bf));
}

int main() {
    printf("Test Case 1: Initialize and check default values\\n");
    struct Task115 bf1;
    Task115_init(&bf1);
    Task115_print(&bf1);
    
    printf("\\nTest Case 2: Set all flags and values\\n");
    struct Task115 bf2;
    Task115_init(&bf2);
    Task115_setFlag1(&bf2, true);
    Task115_setFlag2(&bf2, true);
    Task115_setValue(&bf2, 10);
    Task115_setStatus(&bf2, 2);
    Task115_print(&bf2);
    
    printf("\\nTest Case 3: Set partial values\\n");
    struct Task115 bf3;
    Task115_init(&bf3);
    Task115_setFlag1(&bf3, true);
    Task115_setValue(&bf3, 15);
    Task115_print(&bf3);
    
    printf("\\nTest Case 4: Toggle flags\\n");
    struct Task115 bf4;
    Task115_init(&bf4);
    Task115_setFlag1(&bf4, true);
    Task115_setFlag2(&bf4, false);
    Task115_setValue(&bf4, 7);
    Task115_setStatus(&bf4, 3);
    Task115_print(&bf4);
    Task115_setFlag1(&bf4, false);
    Task115_setFlag2(&bf4, true);
    Task115_print(&bf4);
    
    printf("\\nTest Case 5: Maximum values\\n");
    struct Task115 bf5;
    Task115_init(&bf5);
    Task115_setFlag1(&bf5, true);
    Task115_setFlag2(&bf5, true);
    Task115_setValue(&bf5, 15);
    Task115_setStatus(&bf5, 3);
    Task115_print(&bf5);
    
    return 0;
}
