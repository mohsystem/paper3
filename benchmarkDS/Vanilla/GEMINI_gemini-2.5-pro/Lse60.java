import java.util.Arrays;

public class Lse60 {

    /**
     * Allocates a buffer of a specified size and prints its contents.
     * In Java, a new byte array is automatically initialized with zeros.
     *
     * @param size The size of the buffer to allocate.
     */
    public static void allocateAndPrintBuffer(int size) {
        System.out.println("--- Testing with buffer size: " + size + " ---");
        if (size < 0) {
            System.out.println("Error: Size cannot be negative.");
            System.out.println();
            return;
        }

        try {
            // Allocate a buffer of the specified size.
            byte[] buffer = new byte[size];
            
            // Print the contents of the buffer.
            System.out.println("Allocated buffer contents: " + Arrays.toString(buffer));
        } catch (OutOfMemoryError e) {
            System.out.println("Error: Could not allocate buffer of size " + size + ". Out of memory.");
        }
        System.out.println();
    }

    /**
     * Main method with 5 test cases.
     */
    public static void main(String[] args) {
        allocateAndPrintBuffer(10);
        allocateAndPrintBuffer(0);
        allocateAndPrintBuffer(1);
        allocateAndPrintBuffer(8);
        allocateAndPrintBuffer(5);
    }
}