
public class Lse59 {
    public static void allocateAndFillBuffer(int bufferSize) {
        char[] buffer = new char[bufferSize];
        for (int i = 0; i < bufferSize; i++) {
            buffer[i] = 'A';
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        allocateAndFillBuffer(10);
        System.out.println("Test case 1: Buffer size 10 - Completed");
        
        // Test case 2
        allocateAndFillBuffer(50);
        System.out.println("Test case 2: Buffer size 50 - Completed");
        
        // Test case 3
        allocateAndFillBuffer(100);
        System.out.println("Test case 3: Buffer size 100 - Completed");
        
        // Test case 4
        allocateAndFillBuffer(1);
        System.out.println("Test case 4: Buffer size 1 - Completed");
        
        // Test case 5
        allocateAndFillBuffer(500);
        System.out.println("Test case 5: Buffer size 500 - Completed");
    }
}
