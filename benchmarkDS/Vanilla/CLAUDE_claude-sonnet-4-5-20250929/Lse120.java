
public class Lse120 {
    public static void copyAndPrint(int size, String data) {
        // Allocate two chunks of memory
        char[] chunk1 = new char[size];
        char[] chunk2 = new char[size];
        
        // Copy data into first chunk
        for (int i = 0; i < data.length() && i < size; i++) {
            chunk1[i] = data.charAt(i);
        }
        
        // Copy contents of first chunk into second chunk
        for (int i = 0; i < size; i++) {
            chunk2[i] = chunk1[i];
        }
        
        // Print contents of second chunk
        System.out.println("Contents of second chunk: " + new String(chunk2));
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test Case 1:");
        copyAndPrint(10, "Hello");
        
        // Test case 2
        System.out.println("\\nTest Case 2:");
        copyAndPrint(15, "World");
        
        // Test case 3
        System.out.println("\\nTest Case 3:");
        copyAndPrint(20, "Java Programming");
        
        // Test case 4
        System.out.println("\\nTest Case 4:");
        copyAndPrint(5, "Test");
        
        // Test case 5
        System.out.println("\\nTest Case 5:");
        copyAndPrint(30, "Memory Allocation Example");
    }
}
