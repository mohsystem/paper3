
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;

public class Lse81 {
    private static final int BUFFER_SIZE = 1024;
    private static final int READ_SIZE = 10;
    
    public static char[] allocateBuffer() {
        return new char[BUFFER_SIZE];
    }
    
    public static int readCharacters(char[] buf) {
        if (buf == null || buf.length < READ_SIZE) {
            return -1;
        }
        
        try {
            BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
            int charsRead = 0;
            
            for (int i = 0; i < READ_SIZE; i++) {
                int c = reader.read();
                if (c == -1) {
                    break;
                }
                buf[i] = (char) c;
                charsRead++;
            }
            
            return charsRead;
        } catch (IOException e) {
            return -1;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1:");
        char[] buffer1 = allocateBuffer();
        System.out.println("Buffer allocated with size: " + buffer1.length);
        
        System.out.println("\\nTest Case 2:");
        char[] buffer2 = allocateBuffer();
        System.out.println("Enter 10 characters: ");
        int read = readCharacters(buffer2);
        System.out.println("Characters read: " + read);
        
        System.out.println("\\nTest Case 3: Reading into buffer");
        char[] buffer3 = allocateBuffer();
        for (int i = 0; i < READ_SIZE; i++) {
            buffer3[i] = (char)('A' + i);
        }
        System.out.println("Pre-filled buffer with test data");
        
        System.out.println("\\nTest Case 4: Check buffer bounds");
        char[] buffer4 = allocateBuffer();
        System.out.println("Buffer can safely hold " + READ_SIZE + " characters: " + (buffer4.length >= READ_SIZE));
        
        System.out.println("\\nTest Case 5: Null buffer check");
        int result = readCharacters(null);
        System.out.println("Reading into null buffer returns: " + result);
    }
}
