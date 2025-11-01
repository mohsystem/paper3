
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse79 {
    // Secure memory allocation and input reading with proper bounds checking
    public static String[] memoryAllocationMethod() {
        String[] result = new String[2];
        
        try (BufferedReader reader = new BufferedReader(new InputStreamReader(System.in, StandardCharsets.UTF_8))) {
            // Allocate first buffer (simulated with StringBuilder for safety)
            // Read exactly 100 characters, not 1024 to prevent buffer overflow
            char[] buf = new char[100];
            int bytesRead1 = reader.read(buf, 0, 100);
            
            // Validate read operation succeeded
            if (bytesRead1 < 0) {
                bytesRead1 = 0;
            }
            // Ensure we only use the bytes actually read, preventing out-of-bounds access
            result[0] = new String(buf, 0, bytesRead1);
            
            // Allocate second buffer
            // Read exactly 100 characters, not 1024 to prevent buffer overflow
            char[] buf1 = new char[100];
            int bytesRead2 = reader.read(buf1, 0, 100);
            
            // Validate read operation succeeded
            if (bytesRead2 < 0) {
                bytesRead2 = 0;
            }
            // Ensure we only use the bytes actually read, preventing out-of-bounds access
            result[1] = new String(buf1, 0, bytesRead2);
            
            // Clear sensitive data from buffers
            Arrays.fill(buf, '\\0');
            Arrays.fill(buf1, '\\0');
            
        } catch (IOException e) {
            // Fail closed with generic error message, no internal details leaked
            System.err.println("Input error occurred");
            result[0] = "";
            result[1] = "";
        }
        
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal input
        System.out.println("Test 1: Normal input");
        String[] testResult1 = memoryAllocationMethod();
        System.out.println("Buffer 1 length: " + testResult1[0].length());
        System.out.println("Buffer 2 length: " + testResult1[1].length());
        
        // Test case 2: Empty input
        System.out.println("\\nTest 2: Empty input");
        String[] testResult2 = memoryAllocationMethod();
        System.out.println("Buffer 1 length: " + testResult2[0].length());
        System.out.println("Buffer 2 length: " + testResult2[1].length());
        
        // Test case 3: Short input
        System.out.println("\\nTest 3: Short input");
        String[] testResult3 = memoryAllocationMethod();
        System.out.println("Buffer 1 length: " + testResult3[0].length());
        System.out.println("Buffer 2 length: " + testResult3[1].length());
        
        // Test case 4: Exactly 100 characters
        System.out.println("\\nTest 4: Exactly 100 characters");
        String[] testResult4 = memoryAllocationMethod();
        System.out.println("Buffer 1 length: " + testResult4[0].length());
        System.out.println("Buffer 2 length: " + testResult4[1].length());
        
        // Test case 5: More than 100 characters
        System.out.println("\\nTest 5: More than 100 characters");
        String[] testResult5 = memoryAllocationMethod();
        System.out.println("Buffer 1 length: " + testResult5[0].length());
        System.out.println("Buffer 2 length: " + testResult5[1].length());
    }
}
