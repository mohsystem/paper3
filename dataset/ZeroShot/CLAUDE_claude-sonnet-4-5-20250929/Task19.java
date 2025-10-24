
public class Task19 {
    public static String reverseWords(String str) {
        if (str == null || str.isEmpty()) {
            return str;
        }
        
        String[] words = str.split(" ");
        StringBuilder result = new StringBuilder();
        
        for (int i = 0; i < words.length; i++) {
            if (words[i].length() >= 5) {
                result.append(reverseString(words[i]));
            } else {
                result.append(words[i]);
            }
            
            if (i < words.length - 1) {
                result.append(" ");
            }
        }
        
        return result.toString();
    }
    
    private static String reverseString(String s) {
        return new StringBuilder(s).reverse().toString();
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + reverseWords("Hey fellow warriors"));
        System.out.println("Expected: Hey wollef sroirraw");
        System.out.println();
        
        System.out.println("Test 2: " + reverseWords("This is a test"));
        System.out.println("Expected: This is a test");
        System.out.println();
        
        System.out.println("Test 3: " + reverseWords("This is another test"));
        System.out.println("Expected: This is rehtona test");
        System.out.println();
        
        System.out.println("Test 4: " + reverseWords("abcde"));
        System.out.println("Expected: edcba");
        System.out.println();
        
        System.out.println("Test 5: " + reverseWords("a b c d"));
        System.out.println("Expected: a b c d");
    }
}
