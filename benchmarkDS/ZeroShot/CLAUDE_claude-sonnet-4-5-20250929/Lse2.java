
public class Lse2 {
    public static void copyBytes(byte[] destination, byte[] source, int numBytes) {
        if (destination == null || source == null) {
            throw new IllegalArgumentException("Destination and source cannot be null");
        }
        if (numBytes < 0) {
            throw new IllegalArgumentException("Number of bytes cannot be negative");
        }
        if (numBytes > source.length) {
            throw new IllegalArgumentException("Number of bytes exceeds source length");
        }
        if (numBytes > destination.length) {
            throw new IllegalArgumentException("Number of bytes exceeds destination capacity");
        }
        
        for (int i = 0; i < numBytes; i++) {
            destination[i] = source[i];
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal copy
        byte[] dest1 = new byte[20];
        byte[] src1 = "Hello World".getBytes();
        copyBytes(dest1, src1, src1.length);
        System.out.println("Test 1: " + new String(dest1, 0, src1.length));
        
        // Test case 2: Partial copy
        byte[] dest2 = new byte[10];
        byte[] src2 = "Testing".getBytes();
        copyBytes(dest2, src2, 4);
        System.out.println("Test 2: " + new String(dest2, 0, 4));
        
        // Test case 3: Copy with exact size
        byte[] dest3 = new byte[5];
        byte[] src3 = "Exact".getBytes();
        copyBytes(dest3, src3, 5);
        System.out.println("Test 3: " + new String(dest3));
        
        // Test case 4: Copy zero bytes
        byte[] dest4 = new byte[10];
        byte[] src4 = "Zero".getBytes();
        copyBytes(dest4, src4, 0);
        System.out.println("Test 4: Empty copy successful");
        
        // Test case 5: Copy single byte
        byte[] dest5 = new byte[10];
        byte[] src5 = "A".getBytes();
        copyBytes(dest5, src5, 1);
        System.out.println("Test 5: " + new String(dest5, 0, 1));
    }
}
