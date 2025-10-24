public class Task157 {

    /**
     * Demonstrates dynamic memory allocation and deallocation in Java.
     * In Java, 'new' allocates memory, and the Garbage Collector (GC) automatically
     * deallocates it when the object is no longer reachable.
     * We can suggest an object is ready for collection by removing all references to it.
     *
     * @param size The number of integers to allocate space for.
     */
    public static void manageMemory(int size) {
        if (size <= 0) {
            System.out.println("Size must be positive. Cannot allocate memory for size: " + size);
            return;
        }
        System.out.println("--- Java: Managing memory for size " + size + " ---");

        // 1. Dynamic Allocation: Creating an integer array on the heap.
        int[] dynamicArray = new int[size];
        System.out.println("Allocated an array of " + size + " integers at address hash: " + System.identityHashCode(dynamicArray));

        // 2. Using the allocated memory
        for (int i = 0; i < size; i++) {
            dynamicArray[i] = i * 10;
        }
        System.out.println("Successfully used the allocated memory.");

        // 3. Deallocation: In Java, this is done by making the object eligible for Garbage Collection.
        // We do this by removing the reference to the object.
        dynamicArray = null;
        System.out.println("Memory reference removed. The object is now eligible for garbage collection.");
        System.out.println("------------------------------------------\n");
    }

    public static void main(String[] args) {
        // 5 test cases
        manageMemory(10);
        manageMemory(100);
        manageMemory(5);
        manageMemory(0); // Test case for invalid size
        manageMemory(1000);
    }
}