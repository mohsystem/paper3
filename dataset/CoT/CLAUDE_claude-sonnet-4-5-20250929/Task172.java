
import java.util.HashSet;
import java.util.Set;

public class Task172 {
    public static int distinctEchoSubstrings(String text) {
        // Input validation
        if (text == null || text.isEmpty() || text.length() > 2000) {
            return 0;
        }
        
        // Validate that text contains only lowercase English letters
        for (char c : text.toCharArray()) {
            if (c < 'a' || c > 'z') {
                return 0;
            }
        }
        
        Set<String> distinctSubstrings = new HashSet<>();
        int n = text.length();
        
        // Iterate through all possible substring lengths (must be even)
        for (int len = 2; len <= n; len += 2) {
            int halfLen = len / 2;
            
            // Iterate through all possible starting positions
            for (int i = 0; i <= n - len; i++) {
                // Extract the two halves
                String firstHalf = text.substring(i, i + halfLen);
                String secondHalf = text.substring(i + halfLen, i + len);
                
                // Check if both halves are equal
                if (firstHalf.equals(secondHalf)) {
                    distinctSubstrings.add(firstHalf + secondHalf);
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
        System.out.println("Test 4: " + distinctEchoSubstrings("a")); // Expected: 0
        
        // Test case 5
        System.out.println("Test 5: " + distinctEchoSubstrings("aaaa")); // Expected: 2
    }
}
