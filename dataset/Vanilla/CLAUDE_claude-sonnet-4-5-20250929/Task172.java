
import java.util.*;

public class Task172 {
    public static int distinctEchoSubstrings(String text) {
        Set<String> distinctSubstrings = new HashSet<>();
        int n = text.length();
        
        // Try all possible substring lengths (must be even)
        for (int len = 2; len <= n; len += 2) {
            int halfLen = len / 2;
            
            // Try all possible starting positions
            for (int i = 0; i <= n - len; i++) {
                String first = text.substring(i, i + halfLen);
                String second = text.substring(i + halfLen, i + len);
                
                if (first.equals(second)) {
                    distinctSubstrings.add(first + second);
                }
            }
        }
        
        return distinctSubstrings.size();
    }
    
    public static void main(String[] args) {
        // Test case 1
        String test1 = "abcabcabc";
        System.out.println("Input: \\"" + test1 + "\\"");
        System.out.println("Output: " + distinctEchoSubstrings(test1));
        System.out.println();
        
        // Test case 2
        String test2 = "leetcodeleetcode";
        System.out.println("Input: \\"" + test2 + "\\"");
        System.out.println("Output: " + distinctEchoSubstrings(test2));
        System.out.println();
        
        // Test case 3
        String test3 = "aaaa";
        System.out.println("Input: \\"" + test3 + "\\"");
        System.out.println("Output: " + distinctEchoSubstrings(test3));
        System.out.println();
        
        // Test case 4
        String test4 = "abcdef";
        System.out.println("Input: \\"" + test4 + "\\"");
        System.out.println("Output: " + distinctEchoSubstrings(test4));
        System.out.println();
        
        // Test case 5
        String test5 = "aabbccdd";
        System.out.println("Input: \\"" + test5 + "\\"");
        System.out.println("Output: " + distinctEchoSubstrings(test5));
    }
}
