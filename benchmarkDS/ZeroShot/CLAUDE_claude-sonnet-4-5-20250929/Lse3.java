
public class Lse3 {
    public static void copyBytes(byte[] destination, byte[] source, int numBytes) {
        if (destination == null || source == null) {
            throw new IllegalArgumentException("Destination and source arrays cannot be null");
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
        
        System.arraycopy(source, 0, destination, 0, numBytes);
    }
    
    public static void main(String[] args) {
        // Test case 1: Copy "Hello" to buffer
        byte[] buffer1 = new byte[10];
        byte[] source1 = "Hello".getBytes();
        copyBytes(buffer1, source1, source1.length);
        System.out.println("Test 1: " + new String(buffer1, 0, source1.length));
        
        // Test case 2: Copy partial string
        byte[] buffer2 = new byte[10];
        byte[] source2 = "Hello".getBytes();
        copyBytes(buffer2, source2, 3);
        System.out.println("Test 2: " + new String(buffer2, 0, 3));
        
        // Test case 3: Copy to exact size buffer
        byte[] buffer3 = new byte[5];
        byte[] source3 = "Hello".getBytes();
        copyBytes(buffer3, source3, 5);
        System.out.println("Test 3: " + new String(buffer3));
        
        // Test case 4: Copy empty string
        byte[] buffer4 = new byte[10];
        byte[] source4 = "".getBytes();
        copyBytes(buffer4, source4, 0);
        System.out.println("Test 4: (empty)");
        
        // Test case 5: Copy another string
        byte[] buffer5 = new byte[15];
        byte[] source5 = "World".getBytes();
        copyBytes(buffer5, source5, source5.length);
        System.out.println("Test 5: " + new String(buffer5, 0, source5.length));
    }
}
