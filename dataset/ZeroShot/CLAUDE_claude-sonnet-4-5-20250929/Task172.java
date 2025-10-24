
import java.util.*;

public class Task172 {
    public static int distinctEchoSubstrings(String text) {
        Set<String> distinctSubstrings = new HashSet<>();
        int n = text.length();
        
        // Try all possible substring lengths (must be even)
        for (int len = 2; len <= n; len += 2) {
            int halfLen = len / 2;
            
            // Try all starting positions
            for (int i = 0; i <= n - len; i++) {
                String firstHalf = text.substring(i, i + halfLen);
                String secondHalf = text.substring(i + halfLen, i + len);
                
                if (firstHalf.equals(secondHalf)) {
                    distinctSubstrings.add(text.substring(i, i + len));
                }
            }
        }
        
        return distinctSubstrings.size();
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test 1: " + distinctEchoSubstrings("abcabcabc")); // Expected: 3
        
        // Test case 2
        System.out.println("Test 2: " + distinctEchoSubstrings("leetcodeleetcode")); // Expected: 2
        
        // Test case 3
        System.out.println("Test 3: " + distinctEchoSubstrings("aa")); // Expected: 1
        
        // Test case 4
        System.out.println("Test 4: " + distinctEchoSubstrings("aaaa")); // Expected: 2 ("aa", "aaaa")
        
        // Test case 5
        System.out.println("Test 5: " + distinctEchoSubstrings("abcdefg")); // Expected: 0
    }
}
