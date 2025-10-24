
public class Task191 {
    public static int longestValidParentheses(String s) {
        // Input validation
        if (s == null || s.length() < 2) {
            return 0;
        }
        
        int maxLen = 0;
        int[] dp = new int[s.length()];
        
        for (int i = 1; i < s.length(); i++) {
            if (s.charAt(i) == ')') {
                if (s.charAt(i - 1) == '(') {
                    // Case: ...()
                    dp[i] = (i >= 2 ? dp[i - 2] : 0) + 2;
                } else if (i - dp[i - 1] > 0 && s.charAt(i - dp[i - 1] - 1) == '(') {
                    // Case: ...))
                    dp[i] = dp[i - 1] + 2 + (i - dp[i - 1] >= 2 ? dp[i - dp[i - 1] - 2] : 0);
                }
                maxLen = Math.max(maxLen, dp[i]);
            }
        }
        
        return maxLen;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + longestValidParentheses("(()") + " (Expected: 2)");
        System.out.println("Test 2: " + longestValidParentheses(")()())") + " (Expected: 4)");
        System.out.println("Test 3: " + longestValidParentheses("") + " (Expected: 0)");
        System.out.println("Test 4: " + longestValidParentheses("()(()") + " (Expected: 2)");
        System.out.println("Test 5: " + longestValidParentheses("(()())") + " (Expected: 6)");
    }
}
