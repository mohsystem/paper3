
import java.util.Arrays;

public class Lse1 {
    /**
     * Copies a specified number of bytes from source to destination.
     * 
     * @param destination The destination byte array
     * @param source The source byte array
     * @param numBytes The number of bytes to copy
     * @return true if copy successful, false otherwise
     */
    public static boolean copyBytes(byte[] destination, byte[] source, int numBytes) {
        // Input validation: check for null pointers
        if (destination == null || source == null) {
            return false;
        }
        
        // Input validation: check numBytes is non-negative
        if (numBytes < 0) {
            return false;
        }
        
        // Input validation: prevent integer overflow when checking bounds
        if (numBytes > Integer.MAX_VALUE) {
            return false;
        }
        
        // Boundary check: ensure source has enough bytes to read
        if (numBytes > source.length) {
            return false;
        }
        
        // Boundary check: ensure destination has enough space to write
        if (numBytes > destination.length) {
            return false;
        }
        
        // Safe copy operation using System.arraycopy which performs bounds checking
        try {
            System.arraycopy(source, 0, destination, 0, numBytes);
            return true;
        } catch (IndexOutOfBoundsException | ArrayStoreException | NullPointerException e) {
            // Log error generically without revealing internal details
            return false;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal copy
        byte[] dest1 = new byte[10];
        byte[] src1 = {1, 2, 3, 4, 5};
        boolean result1 = copyBytes(dest1, src1, 5);
        System.out.println("Test 1 - Normal copy: " + result1 + ", Dest: " + Arrays.toString(dest1));
        
        // Test case 2: Copy with numBytes = 0
        byte[] dest2 = new byte[5];
        byte[] src2 = {10, 20, 30};
        boolean result2 = copyBytes(dest2, src2, 0);
        System.out.println("Test 2 - Zero bytes: " + result2 + ", Dest: " + Arrays.toString(dest2));
        
        // Test case 3: Copy more bytes than source has (should fail)
        byte[] dest3 = new byte[10];
        byte[] src3 = {1, 2, 3};
        boolean result3 = copyBytes(dest3, src3, 5);
        System.out.println("Test 3 - Overflow source: " + result3);
        
        // Test case 4: Destination too small (should fail)
        byte[] dest4 = new byte[3];
        byte[] src4 = {1, 2, 3, 4, 5};
        boolean result4 = copyBytes(dest4, src4, 5);
        System.out.println("Test 4 - Overflow destination: " + result4);
        
        // Test case 5: Null source (should fail)
        byte[] dest5 = new byte[5];
        boolean result5 = copyBytes(dest5, null, 3);
        System.out.println("Test 5 - Null source: " + result5);
    }
}
