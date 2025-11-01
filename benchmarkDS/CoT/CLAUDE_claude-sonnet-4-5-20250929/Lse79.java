
import java.util.Scanner;

public class Lse79 {
    public static class BufferResult {
        String buf1;
        String buf2;
        
        BufferResult(String b1, String b2) {
            this.buf1 = b1;
            this.buf2 = b2;
        }
    }
    
    public static BufferResult allocateAndReadBuffers(String input1, String input2) {
        // Security: Limit buffer size to prevent overflow
        final int BUFFER_SIZE = 1024;
        final int READ_SIZE = 100;
        
        // Allocate first buffer (simulated with String in Java)
        StringBuilder buf = new StringBuilder(BUFFER_SIZE);
        
        // Allocate second buffer
        StringBuilder buf1 = new StringBuilder(BUFFER_SIZE);
        
        // Security: Validate and limit input size to prevent overflow
        if (input1 != null) {
            String safeInput1 = input1.length() > READ_SIZE ? 
                               input1.substring(0, READ_SIZE) : input1;
            buf.append(safeInput1);
        }
        
        if (input2 != null) {
            String safeInput2 = input2.length() > READ_SIZE ? 
                               input2.substring(0, READ_SIZE) : input2;
            buf1.append(safeInput2);
        }
        
        return new BufferResult(buf.toString(), buf1.toString());
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal input
        BufferResult result1 = allocateAndReadBuffers("Hello World", "Test Input");
        System.out.println("Test 1 - Buf1: " + result1.buf1 + ", Buf2: " + result1.buf2);
        
        // Test case 2: Empty strings
        BufferResult result2 = allocateAndReadBuffers("", "");
        System.out.println("Test 2 - Buf1: '" + result2.buf1 + "', Buf2: '" + result2.buf2 + "'");
        
        // Test case 3: Null inputs
        BufferResult result3 = allocateAndReadBuffers(null, null);
        System.out.println("Test 3 - Buf1: '" + result3.buf1 + "', Buf2: '" + result3.buf2 + "'");
        
        // Test case 4: Input exceeding 100 characters (should be truncated)
        String longString = "A".repeat(150);
        BufferResult result4 = allocateAndReadBuffers(longString, longString);
        System.out.println("Test 4 - Buf1 length: " + result4.buf1.length() + 
                         ", Buf2 length: " + result4.buf2.length());
        
        // Test case 5: Exactly 100 characters
        String exactString = "B".repeat(100);
        BufferResult result5 = allocateAndReadBuffers(exactString, exactString);
        System.out.println("Test 5 - Buf1 length: " + result5.buf1.length() + 
                         ", Buf2 length: " + result5.buf2.length());
    }
}
