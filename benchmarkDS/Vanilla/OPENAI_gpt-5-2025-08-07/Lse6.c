#include <stdio.h>

const char* get_vehicle_by_index(const char* vehicles[], int length, int index) {
    if (vehicles == NULL || index < 0 || index >= length) return "Invalid index";
    return vehicles[index];
}

void print_vehicles(const char* vehicles[], int length) {
    if (vehicles == NULL) return;
    for (int i = 0; i < length; ++i) {
        printf("%s\n", vehicles[i]);
    }
}

int main(void) {
    const char* vehicles[] = {"Car", "Bus", "Bike", "Truck", "Scooter"};
    int length = sizeof(vehicles) / sizeof(vehicles[0]);

    int testIndices[] = {0, 2, 4, -1, 10};
    int tests = sizeof(testIndices) / sizeof(testIndices[0]);

    for (int i = 0; i < tests; ++i) {
        int idx = testIndices[i];
        const char* result = get_vehicle_by_index(vehicles, length, idx);
        printf("Index %d: %s\n", idx, result);
    }

    printf("All vehicles:\n");
    print_vehicles(vehicles, length);

    return 0;
}