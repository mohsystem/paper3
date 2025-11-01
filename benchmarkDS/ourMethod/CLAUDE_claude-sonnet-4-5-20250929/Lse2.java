
public class Lse2 {
    /**
     * Safely copies a specified number of bytes from source to destination.
     * 
     * @param dest Destination byte array
     * @param src Source byte array
     * @param numBytes Number of bytes to copy
     * @return Number of bytes actually copied, -1 on error
     */
    public static int safeCopy(byte[] dest, byte[] src, int numBytes) {
        // Input validation: check for null pointers
        if (dest == null || src == null) {
            return -1;
        }
        
        // Input validation: check for negative size
        if (numBytes < 0) {
            return -1;
        }
        
        // Bounds checking: ensure numBytes doesn't exceed source array size\n        if (numBytes > src.length) {\n            return -1;\n        }\n        \n        // Bounds checking: ensure numBytes doesn't exceed destination array size
        if (numBytes > dest.length) {
            return -1;
        }
        
        // Safe copy using System.arraycopy which performs bounds checking
        System.arraycopy(src, 0, dest, 0, numBytes);
        
        return numBytes;
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal copy
        byte[] dest1 = new byte[20];
        byte[] src1 = "Hello".getBytes(java.nio.charset.StandardCharsets.UTF_8);
        int result1 = safeCopy(dest1, src1, src1.length);
        System.out.println("Test 1: " + (result1 == src1.length ? "PASS" : "FAIL"));
        
        // Test case 2: Copy with buffer larger than source
        byte[] dest2 = new byte[100];
        byte[] src2 = "Test".getBytes(java.nio.charset.StandardCharsets.UTF_8);
        int result2 = safeCopy(dest2, src2, src2.length);
        System.out.println("Test 2: " + (result2 == src2.length ? "PASS" : "FAIL"));
        
        // Test case 3: Attempt to copy more bytes than source has (should fail)
        byte[] dest3 = new byte[20];
        byte[] src3 = "Hi".getBytes(java.nio.charset.StandardCharsets.UTF_8);
        int result3 = safeCopy(dest3, src3, 10);
        System.out.println("Test 3: " + (result3 == -1 ? "PASS" : "FAIL"));
        
        // Test case 4: Attempt to copy more bytes than destination can hold (should fail)
        byte[] dest4 = new byte[3];
        byte[] src4 = "Hello World".getBytes(java.nio.charset.StandardCharsets.UTF_8);
        int result4 = safeCopy(dest4, src4, src4.length);
        System.out.println("Test 4: " + (result4 == -1 ? "PASS" : "FAIL"));
        
        // Test case 5: Null pointer check
        byte[] dest5 = new byte[10];
        int result5 = safeCopy(dest5, null, 5);
        System.out.println("Test 5: " + (result5 == -1 ? "PASS" : "FAIL"));
    }
}
