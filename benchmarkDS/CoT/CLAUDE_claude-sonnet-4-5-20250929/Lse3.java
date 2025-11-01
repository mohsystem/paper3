
public class Lse3 {
    /**
     * Securely copies bytes from source to destination with bounds checking
     * @param dest destination buffer
     * @param src source buffer
     * @param numBytes number of bytes to copy
     * @return true if successful, false otherwise
     */
    public static boolean secureCopy(byte[] dest, byte[] src, int numBytes) {
        // Null pointer checks
        if (dest == null || src == null) {
            System.out.println("Error: Null buffer provided");
            return false;
        }
        
        // Validate numBytes is non-negative
        if (numBytes < 0) {
            System.out.println("Error: Negative byte count");
            return false;
        }
        
        // Check if source has enough bytes
        if (numBytes > src.length) {
            System.out.println("Error: Source buffer too small");
            return false;
        }
        
        // Check if destination has enough space
        if (numBytes > dest.length) {
            System.out.println("Error: Destination buffer too small");
            return false;
        }
        
        // Perform the copy
        System.arraycopy(src, 0, dest, 0, numBytes);
        return true;
    }
    
    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Copy 'Hello' ===");
        byte[] dest1 = new byte[10];
        byte[] src1 = "Hello".getBytes();
        if (secureCopy(dest1, src1, src1.length)) {
            System.out.println("Result: " + new String(dest1, 0, src1.length));
        }
        
        System.out.println("\\n=== Test Case 2: Buffer too small ===");
        byte[] dest2 = new byte[3];
        byte[] src2 = "Hello".getBytes();
        secureCopy(dest2, src2, src2.length);
        
        System.out.println("\\n=== Test Case 3: Zero bytes ===");
        byte[] dest3 = new byte[10];
        byte[] src3 = "Test".getBytes();
        if (secureCopy(dest3, src3, 0)) {
            System.out.println("Result: Successfully copied 0 bytes");
        }
        
        System.out.println("\\n=== Test Case 4: Null source ===");
        byte[] dest4 = new byte[10];
        secureCopy(dest4, null, 5);
        
        System.out.println("\\n=== Test Case 5: Exact size match ===");
        byte[] dest5 = new byte[5];
        byte[] src5 = "World".getBytes();
        if (secureCopy(dest5, src5, src5.length)) {
            System.out.println("Result: " + new String(dest5));
        }
    }
}
