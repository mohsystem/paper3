public class Lse61 {
    /**
     * In Java, memory is managed by the Garbage Collector (GC).
     * You cannot explicitly free memory like in C/C++.
     * The closest simulation is to make an object eligible for garbage collection
     * by removing all references to it (i.e., setting them to null).
     * Attempting to do this multiple times in a loop is a harmless operation,
     * as subsequent assignments to null on an already-null reference do nothing.
     * This code demonstrates this safe behavior, which prevents double-free vulnerabilities.
     */
    public static void freeBufferInLoop(int bufferSize) {
        if (bufferSize <= 0) {
            System.out.println("Buffer size must be positive.");
            return;
        }

        System.out.println("Allocating buffer of size: " + bufferSize);
        byte[] buffer = new byte[bufferSize];

        System.out.println("Attempting to 'free' the buffer in a loop (by setting reference to null)...");
        for (int i = 0; i < 2; i++) {
            System.out.println("Loop " + (i + 1) + ": Making buffer eligible for GC.");
            // After the first iteration, the reference 'buffer' is already null.
            // This operation is safe and does not cause a crash.
            buffer = null; 
        }
        System.out.println("Operation completed safely.\n");
    }

    public static void main(String[] args) {
        System.out.println("--- Test Case 1 ---");
        freeBufferInLoop(1024);

        System.out.println("--- Test Case 2 ---");
        freeBufferInLoop(256);

        System.out.println("--- Test Case 3 ---");
        freeBufferInLoop(1);

        System.out.println("--- Test Case 4 ---");
        freeBufferInLoop(0);
        
        System.out.println("--- Test Case 5 ---");
        freeBufferInLoop(4096);
    }
}