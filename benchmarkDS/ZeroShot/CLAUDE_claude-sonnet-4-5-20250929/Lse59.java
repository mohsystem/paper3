
public class Lse59 {
    public static void fillBuffer(int bufferSize) {
        char[] buffer = new char[bufferSize];
        
        // Overwrite all values with 'A'
        for (int i = 0; i < bufferSize; i++) {
            buffer[i] = 'A';
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Small buffer
        fillBuffer(10);
        System.out.println("Test case 1: Buffer size 10 - Completed");
        
        // Test case 2: Medium buffer
        fillBuffer(100);
        System.out.println("Test case 2: Buffer size 100 - Completed");
        
        // Test case 3: Large buffer
        fillBuffer(1000);
        System.out.println("Test case 3: Buffer size 1000 - Completed");
        
        // Test case 4: Single element
        fillBuffer(1);
        System.out.println("Test case 4: Buffer size 1 - Completed");
        
        // Test case 5: Empty buffer
        fillBuffer(0);
        System.out.println("Test case 5: Buffer size 0 - Completed");
    }
}
