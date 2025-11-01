
import java.util.Scanner;

public class Lse80 {
    public static int scanString(char[] buffer, int maxSize) {
        if (buffer == null || maxSize <= 0) {
            return -1;
        }
        
        Scanner scanner = new Scanner(System.in);
        String input = scanner.nextLine();
        
        // Limit to maxSize - 1 to leave room for null terminator
        int charsToRead = Math.min(input.length(), maxSize - 1);
        
        for (int i = 0; i < charsToRead; i++) {
            buffer[i] = input.charAt(i);
        }
        buffer[charsToRead] = '\\0';
        
        return charsToRead;
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Short string");
        char[] buffer1 = new char[1024];
        System.out.print("Enter input: ");
        int count1 = scanString(buffer1, 1024);
        System.out.println("Characters read: " + count1);
        System.out.println("Buffer content: " + new String(buffer1, 0, count1));
        
        System.out.println("\\nTest Case 2: Empty string");
        char[] buffer2 = new char[1024];
        System.out.print("Enter input: ");
        int count2 = scanString(buffer2, 1024);
        System.out.println("Characters read: " + count2);
        
        System.out.println("\\nTest Case 3: String with spaces");
        char[] buffer3 = new char[1024];
        System.out.print("Enter input: ");
        int count3 = scanString(buffer3, 1024);
        System.out.println("Characters read: " + count3);
        System.out.println("Buffer content: " + new String(buffer3, 0, count3));
        
        System.out.println("\\nTest Case 4: Maximum length string");
        char[] buffer4 = new char[1024];
        System.out.print("Enter input: ");
        int count4 = scanString(buffer4, 1024);
        System.out.println("Characters read: " + count4);
        
        System.out.println("\\nTest Case 5: Special characters");
        char[] buffer5 = new char[1024];
        System.out.print("Enter input: ");
        int count5 = scanString(buffer5, 1024);
        System.out.println("Characters read: " + count5);
        System.out.println("Buffer content: " + new String(buffer5, 0, count5));
    }
}
