
import java.util.Scanner;

public class Lse80 {
    public static int readString(char[] buffer, int maxSize) {
        Scanner scanner = new Scanner(System.in);
        String input = scanner.nextLine();
        
        int charsToRead = Math.min(input.length(), maxSize - 1);
        
        for (int i = 0; i < charsToRead; i++) {
            buffer[i] = input.charAt(i);
        }
        buffer[charsToRead] = '\\0';
        
        return charsToRead;
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal input
        System.out.println("Test 1 - Enter a string:");
        char[] buffer1 = new char[1024];
        int count1 = readString(buffer1, 1024);
        System.out.println("Characters read: " + count1);
        System.out.println("String: " + new String(buffer1, 0, count1));
        
        // Test case 2: Empty input
        System.out.println("\\nTest 2 - Enter empty string:");
        char[] buffer2 = new char[1024];
        int count2 = readString(buffer2, 1024);
        System.out.println("Characters read: " + count2);
        
        // Test case 3: Short string
        System.out.println("\\nTest 3 - Enter 'Hello':");
        char[] buffer3 = new char[1024];
        int count3 = readString(buffer3, 1024);
        System.out.println("Characters read: " + count3);
        System.out.println("String: " + new String(buffer3, 0, count3));
        
        // Test case 4: Long string
        System.out.println("\\nTest 4 - Enter a long string:");
        char[] buffer4 = new char[1024];
        int count4 = readString(buffer4, 1024);
        System.out.println("Characters read: " + count4);
        System.out.println("String: " + new String(buffer4, 0, count4));
        
        // Test case 5: String with spaces
        System.out.println("\\nTest 5 - Enter 'Hello World':");
        char[] buffer5 = new char[1024];
        int count5 = readString(buffer5, 1024);
        System.out.println("Characters read: " + count5);
        System.out.println("String: " + new String(buffer5, 0, count5));
    }
}
