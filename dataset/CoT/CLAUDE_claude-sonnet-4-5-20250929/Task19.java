
public class Task19 {
    public static String reverseWords(String input) {
        if (input == null) {
            return null;
        }
        
        String[] words = input.split(" ");
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
    
    private static String reverseString(String str) {
        StringBuilder reversed = new StringBuilder();
        for (int i = str.length() - 1; i >= 0; i--) {
            reversed.append(str.charAt(i));
        }
        return reversed.toString();
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println(reverseWords("Hey fellow warriors"));
        
        // Test case 2
        System.out.println(reverseWords("This is a test"));
        
        // Test case 3
        System.out.println(reverseWords("This is another test"));
        
        // Test case 4
        System.out.println(reverseWords("Hello"));
        
        // Test case 5
        System.out.println(reverseWords("Hi"));
    }
}
