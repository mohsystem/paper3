
public class Task141 {
    public static String reverseString(String input) {
        if (input == null) {
            return null;
        }
        return new StringBuilder(input).reverse().toString();
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal string
        System.out.println("Test 1: " + reverseString("hello"));
        
        // Test case 2: String with spaces
        System.out.println("Test 2: " + reverseString("hello world"));
        
        // Test case 3: Empty string
        System.out.println("Test 3: " + reverseString(""));
        
        // Test case 4: Single character
        System.out.println("Test 4: " + reverseString("a"));
        
        // Test case 5: String with special characters
        System.out.println("Test 5: " + reverseString("Hello123!@#"));
    }
}
