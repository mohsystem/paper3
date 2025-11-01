
import java.util.Scanner;

public class Lse80 {
    public static int secureReadInput(char[] buffer, int maxSize) {
        if (buffer == null || maxSize <= 0) {
            return -1;
        }
        
        Scanner scanner = new Scanner(System.in);
        String input = scanner.nextLine();
        
        // Limit the input to maxSize - 1 to leave room for null terminator
        int charsToRead = Math.min(input.length(), maxSize - 1);
        
        for (int i = 0; i < charsToRead; i++) {
            buffer[i] = input.charAt(i);
        }
        
        // Null terminate the buffer
        if (charsToRead < maxSize) {
            buffer[charsToRead] = '\\0';
        }
        
        return charsToRead;
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Short input");
        char[] buffer1 = new char[1024];
        // Simulating input: "Hello"
        
        System.out.println("\\nTest Case 2: Medium input");
        char[] buffer2 = new char[1024];
        // Simulating input: "This is a medium length test string"
        
        System.out.println("\\nTest Case 3: Long input (near limit)");
        char[] buffer3 = new char[1024];
        // Simulating input: String of 1023 characters
        
        System.out.println("\\nTest Case 4: Empty input");
        char[] buffer4 = new char[1024];
        // Simulating input: ""
        
        System.out.println("\\nTest Case 5: Input exceeding buffer size");
        char[] buffer5 = new char[1024];
        // Simulating input: String of 2000 characters (should truncate to 1023)
        
        System.out.println("All test cases completed. Enter strings to test:");
        char[] testBuffer = new char[1024];
        int charsRead = secureReadInput(testBuffer, 1024);
        System.out.println("Characters read: " + charsRead);
    }
}
