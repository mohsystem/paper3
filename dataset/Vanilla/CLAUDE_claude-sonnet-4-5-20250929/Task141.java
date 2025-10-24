
public class Task141 {
    public static String reverseString(String input) {
        if (input == null) {
            return null;
        }
        return new StringBuilder(input).reverse().toString();
    }
    
    public static void main(String[] args) {
        // Test case 1
        String test1 = "Hello";
        System.out.println("Input: " + test1);
        System.out.println("Output: " + reverseString(test1));
        System.out.println();
        
        // Test case 2
        String test2 = "12345";
        System.out.println("Input: " + test2);
        System.out.println("Output: " + reverseString(test2));
        System.out.println();
        
        // Test case 3
        String test3 = "A man a plan a canal Panama";
        System.out.println("Input: " + test3);
        System.out.println("Output: " + reverseString(test3));
        System.out.println();
        
        // Test case 4
        String test4 = "";
        System.out.println("Input: \\"" + test4 + "\\"");
        System.out.println("Output: \\"" + reverseString(test4) + "\\"");
        System.out.println();
        
        // Test case 5
        String test5 = "Reverse Me!";
        System.out.println("Input: " + test5);
        System.out.println("Output: " + reverseString(test5));
    }
}
