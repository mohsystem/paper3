
public class Lse1 {
    public static void copyBytes(byte[] destination, byte[] source, int numBytes) {
        if (destination == null || source == null) {
            throw new IllegalArgumentException("Destination and source arrays cannot be null");
        }
        
        if (numBytes < 0) {
            throw new IllegalArgumentException("Number of bytes cannot be negative");
        }
        
        if (numBytes > source.length) {
            throw new IllegalArgumentException("Number of bytes exceeds source array length");
        }
        
        if (numBytes > destination.length) {
            throw new IllegalArgumentException("Number of bytes exceeds destination array length");
        }
        
        System.arraycopy(source, 0, destination, 0, numBytes);
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal copy
        byte[] src1 = {1, 2, 3, 4, 5};
        byte[] dst1 = new byte[5];
        copyBytes(dst1, src1, 5);
        System.out.println("Test 1: " + java.util.Arrays.toString(dst1));
        
        // Test case 2: Partial copy
        byte[] src2 = {10, 20, 30, 40, 50};
        byte[] dst2 = new byte[5];
        copyBytes(dst2, src2, 3);
        System.out.println("Test 2: " + java.util.Arrays.toString(dst2));
        
        // Test case 3: Copy 0 bytes
        byte[] src3 = {1, 2, 3};
        byte[] dst3 = new byte[3];
        copyBytes(dst3, src3, 0);
        System.out.println("Test 3: " + java.util.Arrays.toString(dst3));
        
        // Test case 4: Single byte copy
        byte[] src4 = {99};
        byte[] dst4 = new byte[1];
        copyBytes(dst4, src4, 1);
        System.out.println("Test 4: " + java.util.Arrays.toString(dst4));
        
        // Test case 5: Larger array
        byte[] src5 = {11, 22, 33, 44, 55, 66, 77, 88};
        byte[] dst5 = new byte[8];
        copyBytes(dst5, src5, 8);
        System.out.println("Test 5: " + java.util.Arrays.toString(dst5));
    }
}
