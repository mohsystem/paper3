import java.util.ArrayList;

public class Task157 {

    /**
     * Demonstrates dynamic memory allocation in Java.
     * In Java, memory is allocated with 'new' and deallocated automatically by the Garbage Collector (GC)
     * when an object is no longer reachable. Setting a reference to null is a way to suggest to the GC
     * that the object can be collected.
     *
     * @param size The number of Integer objects to allocate in a list.
     */
    public static void demonstrateResourceManagement(int size) {
        if (size <= 0) {
            System.out.println("Size must be positive. No allocation performed.");
            return;
        }

        ArrayList<Integer> dynamicallyAllocatedList = null;
        try {
            // 1. Dynamic Allocation: Creating an ArrayList object.
            dynamicallyAllocatedList = new ArrayList<>(size);
            for (int i = 0; i < size; i++) {
                dynamicallyAllocatedList.add(i);
            }
            System.out.println("Successfully allocated an ArrayList with capacity for " + size + " integers.");
            // Use the resource...
            System.out.println("First element: " + dynamicallyAllocatedList.get(0));

        } catch (OutOfMemoryError e) {
            System.err.println("Memory allocation failed for size " + size + ": " + e.getMessage());
        } finally {
            // 2. Deallocation (making eligible for GC):
            // By setting the only reference to the object to null, we make it
            // unreachable and thus eligible for garbage collection.
            dynamicallyAllocatedList = null;
            System.out.println("Resource reference cleared. Object is now eligible for garbage collection.");
            // Calling System.gc() is a suggestion, not a command. The GC runs at its own discretion.
            // System.gc();
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java Dynamic Memory Management ---");
        // 5 Test Cases
        demonstrateResourceManagement(10);
        System.out.println("--------------------");
        demonstrateResourceManagement(1000);
        System.out.println("--------------------");
        demonstrateResourceManagement(0);
        System.out.println("--------------------");
        demonstrateResourceManagement(50000);
        System.out.println("--------------------");
        // This large allocation might fail depending on heap size
        demonstrateResourceManagement(Integer.MAX_VALUE / 10);
        System.out.println("--- End of Java Demo ---");
    }
}