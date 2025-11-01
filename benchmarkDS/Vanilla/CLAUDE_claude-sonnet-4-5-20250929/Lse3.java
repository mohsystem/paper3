
public class Lse3 {
    public static void memcpy(byte[] dest, byte[] src, int numBytes) {
        for (int i = 0; i < numBytes && i < src.length && i < dest.length; i++) {
            dest[i] = src[i];
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Copy "Hello"
        byte[] buffer1 = new byte[10];
        byte[] source1 = "Hello".getBytes();
        memcpy(buffer1, source1, 5);
        System.out.println("Test 1: " + new String(buffer1, 0, 5));
        
        // Test case 2: Copy "World"
        byte[] buffer2 = new byte[10];
        byte[] source2 = "World".getBytes();
        memcpy(buffer2, source2, 5);
        System.out.println("Test 2: " + new String(buffer2, 0, 5));
        
        // Test case 3: Copy partial string
        byte[] buffer3 = new byte[10];
        byte[] source3 = "Programming".getBytes();
        memcpy(buffer3, source3, 7);
        System.out.println("Test 3: " + new String(buffer3, 0, 7));
        
        // Test case 4: Copy empty string
        byte[] buffer4 = new byte[10];
        byte[] source4 = "".getBytes();
        memcpy(buffer4, source4, 0);
        System.out.println("Test 4: " + new String(buffer4, 0, 0));
        
        // Test case 5: Copy single character
        byte[] buffer5 = new byte[10];
        byte[] source5 = "A".getBytes();
        memcpy(buffer5, source5, 1);
        System.out.println("Test 5: " + new String(buffer5, 0, 1));
    }
}
