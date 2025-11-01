
public class Lse3 {
    /**
     * Copies bytes from source to destination with bounds checking.
     * @param dest Destination buffer
     * @param src Source buffer
     * @param numBytes Number of bytes to copy
     * @return true if copy successful, false otherwise
     */
    public static boolean safeCopy(byte[] dest, byte[] src, int numBytes) {
        // Validate all inputs are non-null to prevent NullPointerException
        if (dest == null || src == null) {
            return false;
        }
        
        // Validate numBytes is non-negative to prevent integer underflow
        if (numBytes < 0) {
            return false;
        }
        
        // Validate numBytes does not exceed source length to prevent out-of-bounds read
        if (numBytes > src.length) {
            return false;
        }
        
        // Validate numBytes does not exceed destination length to prevent buffer overflow
        if (numBytes > dest.length) {
            return false;
        }
        
        // Perform safe copy using System.arraycopy which includes bounds checking
        System.arraycopy(src, 0, dest, 0, numBytes);
        return true;
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal copy of "Hello"
        byte[] buffer1 = new byte[10];
        byte[] hello = "Hello".getBytes(java.nio.charset.StandardCharsets.UTF_8);
        if (safeCopy(buffer1, hello, hello.length)) {
            System.out.println("Test 1 passed: " + new String(buffer1, 0, hello.length, java.nio.charset.StandardCharsets.UTF_8));
        }
        
        // Test case 2: Destination buffer too small (should fail safely)
        byte[] buffer2 = new byte[3];
        if (!safeCopy(buffer2, hello, hello.length)) {
            System.out.println("Test 2 passed: Correctly rejected overflow");
        }
        
        // Test case 3: Copy zero bytes (edge case)
        byte[] buffer3 = new byte[10];
        if (safeCopy(buffer3, hello, 0)) {
            System.out.println("Test 3 passed: Zero byte copy succeeded");
        }
        
        // Test case 4: Null destination (should fail safely)
        if (!safeCopy(null, hello, hello.length)) {
            System.out.println("Test 4 passed: Correctly rejected null destination");
        }
        
        // Test case 5: Negative byte count (should fail safely)
        byte[] buffer5 = new byte[10];
        if (!safeCopy(buffer5, hello, -1)) {
            System.out.println("Test 5 passed: Correctly rejected negative count");
        }
    }
}
