public class Task157 {

    /**
     * Demonstrates dynamic memory allocation and deallocation in Java.
     * In Java, memory is allocated using 'new' and deallocated automatically by the Garbage Collector
     * when an object is no longer reachable. We can suggest deallocation by removing all references to the object.
     *
     * @param size The number of integers to allocate space for.
     */
    public static void manageMemory(int size) {
        if (size <= 0) {
            System.out.println("Cannot allocate memory for size " + size + ".");
            return;
        }

        System.out.println("Attempting to allocate memory for an array of " + size + " integers.");
        
        // 1. Dynamic Allocation
        int[] dynamicArray = new int[size];
        System.out.println("Memory allocated successfully at reference: " + dynamicArray);

        // Using the allocated memory
        dynamicArray[0] = 100;
        System.out.println("First element set to: " + dynamicArray[0]);

        // 2. Dynamic Deallocation (making the object eligible for Garbage Collection)
        System.out.println("Removing reference to the array, making it eligible for garbage collection.");
        dynamicArray = null; 
        
        // We can suggest that the JVM run the garbage collector, but it's not guaranteed.
        // System.gc(); 
        
        System.out.println("Memory for " + size + " integers is now eligible to be freed.\n");
    }

    public static void main(String[] args) {
        System.out.println("--- Java Memory Management Demo ---");
        // 5 Test Cases
        manageMemory(10);
        manageMemory(1000);
        manageMemory(50000);
        manageMemory(0);
        manageMemory(1);
        System.out.println("--- End of Java Demo ---\n");
    }
}