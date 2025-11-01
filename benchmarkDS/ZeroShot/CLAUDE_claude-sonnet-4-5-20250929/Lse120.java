
public class Lse120 {
    public static void copyAndPrint(String source) {
        if (source == null) {
            System.out.println("Source is null");
            return;
        }
        
        // Allocate first chunk and store data
        String firstChunk = new String(source);
        
        // Allocate second chunk and copy data from first chunk
        String secondChunk = new String(firstChunk);
        
        // Print contents of second chunk
        System.out.println(secondChunk);
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal string
        System.out.println("Test 1:");
        copyAndPrint("Hello, World!");
        
        // Test case 2: Empty string
        System.out.println("\\nTest 2:");
        copyAndPrint("");
        
        // Test case 3: String with special characters
        System.out.println("\\nTest 3:");
        copyAndPrint("Special @#$% Characters!");
        
        // Test case 4: Null input
        System.out.println("\\nTest 4:");
        copyAndPrint(null);
        
        // Test case 5: Long string
        System.out.println("\\nTest 5:");
        copyAndPrint("This is a longer string with multiple words and spaces");
    }
}
