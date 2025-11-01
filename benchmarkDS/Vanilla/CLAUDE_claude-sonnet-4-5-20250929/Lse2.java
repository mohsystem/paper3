
public class Lse2 {
    public static void copyBytes(byte[] destination, byte[] source, int numBytes) {
        for (int i = 0; i < numBytes && i < source.length && i < destination.length; i++) {
            destination[i] = source[i];
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal copy
        byte[] dest1 = new byte[20];
        byte[] src1 = "Hello World".getBytes();
        copyBytes(dest1, src1, src1.length);
        System.out.println("Test 1: " + new String(dest1).trim());
        
        // Test case 2: Copy fewer bytes than source
        byte[] dest2 = new byte[20];
        byte[] src2 = "Hello World".getBytes();
        copyBytes(dest2, src2, 5);
        System.out.println("Test 2: " + new String(dest2).trim());
        
        // Test case 3: Empty source
        byte[] dest3 = new byte[20];
        byte[] src3 = "".getBytes();
        copyBytes(dest3, src3, 0);
        System.out.println("Test 3: " + new String(dest3).trim());
        
        // Test case 4: Copy with exact buffer size
        byte[] dest4 = new byte[5];
        byte[] src4 = "12345".getBytes();
        copyBytes(dest4, src4, 5);
        System.out.println("Test 4: " + new String(dest4));
        
        // Test case 5: Larger numBytes than source
        byte[] dest5 = new byte[20];
        byte[] src5 = "Test".getBytes();
        copyBytes(dest5, src5, 10);
        System.out.println("Test 5: " + new String(dest5).trim());
    }
}
