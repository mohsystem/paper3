
#include <stdio.h>
#include <stdlib.h>

// Note: C doesn't have built-in heap/priority queue data structures.
// This implementation uses a simple sorted array approach for demonstration.
// For production use with large datasets, implement a proper heap structure.

typedef struct {
    int* data;
    int size;
    int capacity;
} Task195;

Task195* Task195Create() {
    Task195* obj = (Task195*)malloc(sizeof(Task195));
    obj->capacity = 10000;
    obj->data = (int*)malloc(obj->capacity * sizeof(int));
    obj->size = 0;
    return obj;
}

void Task195AddNum(Task195* obj, int num) {
    if (obj->size >= obj->capacity) {
        obj->capacity *= 2;
        obj->data = (int*)realloc(obj->data, obj->capacity * sizeof(int));
    }
    
    // Insert in sorted position
    int i = obj->size - 1;
    while (i >= 0 && obj->data[i] > num) {
        obj->data[i + 1] = obj->data[i];
        i--;
    }
    obj->data[i + 1] = num;
    obj->size++;
}

double Task195FindMedian(Task195* obj) {
    if (obj->size % 2 == 0) {
        int mid1 = obj->size / 2 - 1;
        int mid2 = obj->size / 2;
        return (obj->data[mid1] + obj->data[mid2]) / 2.0;
    }
    return obj->data[obj->size / 2];
}

void Task195Free(Task195* obj) {
    free(obj->data);
    free(obj);
}

int main() {
    // Test case 1
    Task195* mf1 = Task195Create();
    Task195AddNum(mf1, 1);
    Task195AddNum(mf1, 2);
    printf("Test 1: %.1f\\n", Task195FindMedian(mf1)); // 1.5
    Task195AddNum(mf1, 3);
    printf("Test 1: %.1f\\n", Task195FindMedian(mf1)); // 2.0
    Task195Free(mf1);
    
    // Test case 2
    Task195* mf2 = Task195Create();
    Task195AddNum(mf2, 5);
    printf("Test 2: %.1f\\n", Task195FindMedian(mf2)); // 5.0
    Task195Free(mf2);
    
    // Test case 3
    Task195* mf3 = Task195Create();
    Task195AddNum(mf3, 6);
    Task195AddNum(mf3, 10);
    Task195AddNum(mf3, 2);
    Task195AddNum(mf3, 6);
    Task195AddNum(mf3, 5);
    printf("Test 3: %.1f\\n", Task195FindMedian(mf3)); // 6.0
    Task195Free(mf3);
    
    // Test case 4
    Task195* mf4 = Task195Create();
    Task195AddNum(mf4, -1);
    Task195AddNum(mf4, -2);
    Task195AddNum(mf4, -3);
    Task195AddNum(mf4, -4);
    printf("Test 4: %.1f\\n", Task195FindMedian(mf4)); // -2.5
    Task195Free(mf4);
    
    // Test case 5
    Task195* mf5 = Task195Create();
    Task195AddNum(mf5, 1);
    Task195AddNum(mf5, 2);
    Task195AddNum(mf5, 3);
    Task195AddNum(mf5, 4);
    Task195AddNum(mf5, 5);
    printf("Test 5: %.1f\\n", Task195FindMedian(mf5)); // 3.0
    Task195Free(mf5);
    
    return 0;
}
