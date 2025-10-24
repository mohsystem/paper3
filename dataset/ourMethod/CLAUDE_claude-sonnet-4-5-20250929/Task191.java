
import java.nio.charset.StandardCharsets;

public class Task191 {
    public static int longestValidParentheses(String s) {
        // Validate input
        if (s == null) {
            return 0;
        }
        
        // Validate length constraint
        if (s.length() > 30000) {
            throw new IllegalArgumentException("Input length exceeds maximum allowed");
        }
        
        // Validate characters
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c != '(' && c != ')') {
                throw new IllegalArgumentException("Invalid character in input");
            }
        }
        
        int maxLen = 0;
        int[] dp = new int[s.length()];
        
        for (int i = 1; i < s.length(); i++) {
            if (s.charAt(i) == ')') {
                if (s.charAt(i - 1) == '(') {
                    dp[i] = (i >= 2 ? dp[i - 2] : 0) + 2;
                } else if (i - dp[i - 1] > 0 && s.charAt(i - dp[i - 1] - 1) == '(') {
                    dp[i] = dp[i - 1] + 2 + ((i - dp[i - 1]) >= 2 ? dp[i - dp[i - 1] - 2] : 0);
                }
                maxLen = Math.max(maxLen, dp[i]);
            }
        }
        
        return maxLen;
    }
    
    public static void main(String[] args) {
        // Test case 1
        String test1 = "(()";
        System.out.println("Input: \\"" + test1 + "\\" => Output: " + longestValidParentheses(test1));
        
        // Test case 2
        String test2 = ")()())";
        System.out.println("Input: \\"" + test2 + "\\" => Output: " + longestValidParentheses(test2));
        
        // Test case 3
        String test3 = "";
        System.out.println("Input: \\"" + test3 + "\\" => Output: " + longestValidParentheses(test3));
        
        // Test case 4
        String test4 = "((()))";
        System.out.println("Input: \\"" + test4 + "\\" => Output: " + longestValidParentheses(test4));
        
        // Test case 5
        String test5 = "()(())";
        System.out.println("Input: \\"" + test5 + "\\" => Output: " + longestValidParentheses(test5));
    }
}
