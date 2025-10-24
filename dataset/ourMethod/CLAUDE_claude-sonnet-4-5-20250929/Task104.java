
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Task104 {
    private static final int BUFFER_SIZE = 256;
    
    public static class BufferResult {
        public final boolean success;
        public final String message;
        public final byte[] data;
        
        public BufferResult(boolean success, String message, byte[] data) {
            this.success = success;
            this.message = message;
            this.data = data != null ? Arrays.copyOf(data, data.length) : null;
        }
    }
    
    public static BufferResult handleUserInput(String input, int maxBufferSize) {
        if (input == null) {
            return new BufferResult(false, "Input cannot be null", null);
        }
        
        if (maxBufferSize <= 0 || maxBufferSize > 1048576) {
            return new BufferResult(false, "Invalid buffer size", null);
        }
        
        byte[] inputBytes = input.getBytes(StandardCharsets.UTF_8);
        
        if (inputBytes.length > maxBufferSize) {
            return new BufferResult(false, 
                String.format("Input exceeds buffer size limit (%d > %d)", 
                    inputBytes.length, maxBufferSize), null);
        }
        
        byte[] buffer = new byte[maxBufferSize];
        Arrays.fill(buffer, (byte)0);
        
        System.arraycopy(inputBytes, 0, buffer, 0, inputBytes.length);
        
        byte[] result = Arrays.copyOf(buffer, inputBytes.length);
        return new BufferResult(true, "Input handled successfully", result);
    }
    
    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Valid short input ===");
        BufferResult result1 = handleUserInput("Hello World", BUFFER_SIZE);
        System.out.println("Success: " + result1.success);
        System.out.println("Message: " + result1.message);
        if (result1.data != null) {
            System.out.println("Data: " + new String(result1.data, StandardCharsets.UTF_8));
        }
        
        System.out.println("\\n=== Test Case 2: Empty input ===");
        BufferResult result2 = handleUserInput("", BUFFER_SIZE);
        System.out.println("Success: " + result2.success);
        System.out.println("Message: " + result2.message);
        
        System.out.println("\\n=== Test Case 3: Input at buffer limit ===");
        char[] maxChars = new char[BUFFER_SIZE];
        Arrays.fill(maxChars, 'A');
        String maxInput = new String(maxChars);
        BufferResult result3 = handleUserInput(maxInput, BUFFER_SIZE);
        System.out.println("Success: " + result3.success);
        System.out.println("Message: " + result3.message);
        if (result3.data != null) {
            System.out.println("Data length: " + result3.data.length);
        }
        
        System.out.println("\\n=== Test Case 4: Input exceeding buffer size ===");
        char[] oversizeChars = new char[BUFFER_SIZE + 10];
        Arrays.fill(oversizeChars, 'B');
        String oversizeInput = new String(oversizeChars);
        BufferResult result4 = handleUserInput(oversizeInput, BUFFER_SIZE);
        System.out.println("Success: " + result4.success);
        System.out.println("Message: " + result4.message);
        
        System.out.println("\\n=== Test Case 5: Null input ===");
        BufferResult result5 = handleUserInput(null, BUFFER_SIZE);
        System.out.println("Success: " + result5.success);
        System.out.println("Message: " + result5.message);
    }
}
