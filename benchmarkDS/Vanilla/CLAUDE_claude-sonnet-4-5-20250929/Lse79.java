
import java.util.Scanner;

public class Lse79 {
    public static void memoryAllocationMethod() {
        Scanner scanner = new Scanner(System.in);
        
        // Allocate first buffer of 1024 bytes
        char[] buf = new char[1024];
        
        // Read up to 100 characters from stdin into first buffer
        String input1 = scanner.nextLine();
        int len1 = Math.min(input1.length(), 100);
        for (int i = 0; i < len1; i++) {
            buf[i] = input1.charAt(i);
        }
        
        // Allocate second buffer of 1024 bytes
        char[] buf1 = new char[1024];
        
        // Read up to 100 characters from stdin into second buffer
        String input2 = scanner.nextLine();
        int len2 = Math.min(input2.length(), 100);
        for (int i = 0; i < len2; i++) {
            buf1[i] = input2.charAt(i);
        }
        
        System.out.println("First buffer (first 100 chars): " + new String(buf, 0, len1));
        System.out.println("Second buffer (first 100 chars): " + new String(buf1, 0, len2));
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1:");
        System.out.println("Enter first input: Hello World");
        System.out.println("Enter second input: Java Programming");
        memoryAllocationMethod();
        
        System.out.println("\\nTest Case 2:");
        System.out.println("Enter first input: This is a test string");
        System.out.println("Enter second input: Another test string");
        memoryAllocationMethod();
        
        System.out.println("\\nTest Case 3:");
        System.out.println("Enter first input: 12345");
        System.out.println("Enter second input: 67890");
        memoryAllocationMethod();
        
        System.out.println("\\nTest Case 4:");
        System.out.println("Enter first input: " + "A".repeat(150));
        System.out.println("Enter second input: " + "B".repeat(150));
        memoryAllocationMethod();
        
        System.out.println("\\nTest Case 5:");
        System.out.println("Enter first input: ");
        System.out.println("Enter second input: ");
        memoryAllocationMethod();
    }
}
