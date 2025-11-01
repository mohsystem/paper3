
import java.util.Scanner;

public class Lse81 {
    private static final int BUFFER_SIZE = 1024;
    private static final int READ_SIZE = 10;
    
    public static byte[] allocateBuffer() {
        return new byte[BUFFER_SIZE];
    }
    
    public static int readCharacters(byte[] buffer) {
        if (buffer == null || buffer.length < READ_SIZE) {
            return -1;
        }
        
        Scanner scanner = new Scanner(System.in);
        String input = scanner.nextLine();
        
        int charsToRead = Math.min(READ_SIZE, input.length());
        charsToRead = Math.min(charsToRead, buffer.length);
        
        for (int i = 0; i < charsToRead; i++) {
            buffer[i] = (byte) input.charAt(i);
        }
        
        return charsToRead;
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal input
        System.out.println("Test 1 - Enter 10 characters:");
        byte[] buf1 = allocateBuffer();
        int read1 = readCharacters(buf1);
        System.out.println("Characters read: " + read1);
        
        // Test case 2: Less than 10 characters
        System.out.println("\\nTest 2 - Enter less than 10 characters:");
        byte[] buf2 = allocateBuffer();
        int read2 = readCharacters(buf2);
        System.out.println("Characters read: " + read2);
        
        // Test case 3: More than 10 characters (should only read 10)
        System.out.println("\\nTest 3 - Enter more than 10 characters:");
        byte[] buf3 = allocateBuffer();
        int read3 = readCharacters(buf3);
        System.out.println("Characters read: " + read3);
        
        // Test case 4: Empty input
        System.out.println("\\nTest 4 - Press Enter without input:");
        byte[] buf4 = allocateBuffer();
        int read4 = readCharacters(buf4);
        System.out.println("Characters read: " + read4);
        
        // Test case 5: Special characters
        System.out.println("\\nTest 5 - Enter special characters:");
        byte[] buf5 = allocateBuffer();
        int read5 = readCharacters(buf5);
        System.out.println("Characters read: " + read5);
    }
}
