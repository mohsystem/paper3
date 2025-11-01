
public class Lse1 {
    /**
     * Safely copies bytes from source to destination
     * @param dest Destination byte array
     * @param src Source byte array
     * @param numBytes Number of bytes to copy
     * @return true if successful, false otherwise
     */
    public static boolean safeCopyBytes(byte[] dest, byte[] src, int numBytes) {
        // Input validation
        if (dest == null || src == null) {
            System.err.println("Error: Null pointer provided");
            return false;
        }
        
        if (numBytes < 0) {
            System.err.println("Error: Negative byte count");
            return false;
        }
        
        if (numBytes == 0) {
            return true; // Nothing to copy
        }
        
        // Bounds checking
        if (numBytes > src.length) {
            System.err.println("Error: Source buffer too small");
            return false;
        }
        
        if (numBytes > dest.length) {
            System.err.println("Error: Destination buffer too small");
            return false;
        }
        
        // Safe copy operation
        System.arraycopy(src, 0, dest, 0, numBytes);
        return true;
    }
    
    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Normal copy ===");
        byte[] src1 = {1, 2, 3, 4, 5};
        byte[] dest1 = new byte[5];
        boolean result1 = safeCopyBytes(dest1, src1, 5);
        System.out.println("Result: " + result1 + ", Dest: " + java.util.Arrays.toString(dest1));
        
        System.out.println("\\n=== Test Case 2: Partial copy ===");
        byte[] src2 = {10, 20, 30, 40, 50};
        byte[] dest2 = new byte[10];
        boolean result2 = safeCopyBytes(dest2, src2, 3);
        System.out.println("Result: " + result2 + ", Dest: " + java.util.Arrays.toString(dest2));
        
        System.out.println("\\n=== Test Case 3: Null source ===");
        byte[] dest3 = new byte[5];
        boolean result3 = safeCopyBytes(dest3, null, 5);
        System.out.println("Result: " + result3);
        
        System.out.println("\\n=== Test Case 4: Buffer overflow attempt ===");
        byte[] src4 = {1, 2, 3};
        byte[] dest4 = new byte[2];
        boolean result4 = safeCopyBytes(dest4, src4, 3);
        System.out.println("Result: " + result4);
        
        System.out.println("\\n=== Test Case 5: Zero bytes ===");
        byte[] src5 = {1, 2, 3};
        byte[] dest5 = new byte[3];
        boolean result5 = safeCopyBytes(dest5, src5, 0);
        System.out.println("Result: " + result5 + ", Dest: " + java.util.Arrays.toString(dest5));
    }
}
