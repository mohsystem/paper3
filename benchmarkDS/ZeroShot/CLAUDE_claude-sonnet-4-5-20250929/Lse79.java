
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;

public class Lse79 {
    public static class MemoryBuffer {
        private char[] buffer;
        private int size;
        
        public MemoryBuffer(int size) {
            this.size = size;
            this.buffer = new char[size];
        }
        
        public void readInput(int maxChars) throws IOException {
            BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));
            int charsToRead = Math.min(maxChars, size - 1);
            int charsRead = reader.read(buffer, 0, charsToRead);
            if (charsRead > 0) {
                buffer[charsRead] = '\\0';
            }
        }
        
        public String getContent() {
            return new String(buffer).trim();
        }
    }
    
    public static void allocateAndReadBuffers() throws IOException {
        MemoryBuffer buf = new MemoryBuffer(1024);
        buf.readInput(100);
        
        MemoryBuffer buf1 = new MemoryBuffer(1024);
        buf1.readInput(100);
        
        System.out.println("Buffer 1: " + buf.getContent());
        System.out.println("Buffer 2: " + buf1.getContent());
    }
    
    public static void main(String[] args) {
        System.out.println("Test cases for memory allocation and input reading:");
        
        // Note: For automated testing, we would need to mock stdin
        // These test cases demonstrate the structure
        try {
            System.out.println("\\nTest Case 1: Normal execution");
            allocateAndReadBuffers();
        } catch (IOException e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
