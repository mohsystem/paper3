import java.util.Arrays;

public class Task157 {

    /**
     * Dynamically allocates an array of integers, uses it, and then marks it for
     * garbage collection.
     *
     * @param size The number of integers to allocate.
     */
    public static void demonstrateAllocation(int size) {
        if (size < 0) {
            System.out.println("Error: Cannot allocate a negative-sized array.");
            return;
        }
        if (size == 0) {
            System.out.println("Allocating a zero-sized array.");
        }

        System.out.println("Attempting to allocate memory for " + size + " integers...");

        int[] data = null;
        try {
            // 1. Dynamic Allocation
            data = new int[size];
            System.out.println("Successfully allocated " + (size * 4) + " bytes.");

            // 2. Use the allocated memory
            for (int i = 0; i < size; i++) {
                data[i] = i * 2;
            }
            if (size > 0) {
               System.out.println("Used memory. First element: " + data[0] + ", Last element: " + data[size - 1]);
            } else {
               System.out.println("Used memory. Array is empty.");
            }

        } catch (OutOfMemoryError e) {
            System.err.println("Error: Memory allocation failed. " + e.getMessage());
        } catch (NegativeArraySizeException e) {
            System.err.println("Error: Invalid size for allocation. " + e.getMessage());
        } finally {
            // 3. Deallocation (in Java, this is done by the Garbage Collector)
            // We can make the object eligible for collection by removing all references to it.
            data = null;
            System.out.println("Memory reference released. The resource is now eligible for garbage collection.");
            System.out.println("----------------------------------------");
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java Dynamic Memory Demonstration ---");
        // Test case 1: A small, valid size
        demonstrateAllocation(10);
        // Test case 2: A larger size
        demonstrateAllocation(100000);
        // Test case 3: A zero size
        demonstrateAllocation(0);
        // Test case 4: A negative size (invalid)
        demonstrateAllocation(-5);
        // Test case 5: A very large size that might cause an OutOfMemoryError
        demonstrateAllocation(Integer.MAX_VALUE / 2);
    }
}