
public class Lse2 {
    public static boolean secureCopy(byte[] destination, byte[] source, int numBytes) {
        // Input validation
        if (destination == null || source == null) {
            System.err.println("Error: Null pointer provided");
            return false;
        }
        
        if (numBytes < 0) {
            System.err.println("Error: Negative byte count");
            return false;
        }
        
        if (numBytes > source.length) {
            System.err.println("Error: Copy size exceeds source length");
            return false;
        }
        
        if (numBytes > destination.length) {
            System.err.println("Error: Copy size exceeds destination capacity");
            return false;
        }
        
        // Safe copy operation
        System.arraycopy(source, 0, destination, 0, numBytes);
        return true;
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal copy
        byte[] dest1 = new byte[20];
        byte[] src1 = "Hello World".getBytes();
        System.out.println("Test 1: " + secureCopy(dest1, src1, src1.length));
        System.out.println("Result: " + new String(dest1, 0, src1.length));
        
        // Test case 2: Partial copy
        byte[] dest2 = new byte[10];
        byte[] src2 = "Test".getBytes();
        System.out.println("\\nTest 2: " + secureCopy(dest2, src2, 4));
        System.out.println("Result: " + new String(dest2, 0, 4));
        
        // Test case 3: Buffer overflow prevention
        byte[] dest3 = new byte[5];
        byte[] src3 = "Too long message".getBytes();
        System.out.println("\\nTest 3: " + secureCopy(dest3, src3, src3.length));
        
        // Test case 4: Null source
        byte[] dest4 = new byte[10];
        System.out.println("\\nTest 4: " + secureCopy(dest4, null, 5));
        
        // Test case 5: Negative byte count
        byte[] dest5 = new byte[10];
        byte[] src5 = "Data".getBytes();
        System.out.println("\\nTest 5: " + secureCopy(dest5, src5, -1));
    }
}
