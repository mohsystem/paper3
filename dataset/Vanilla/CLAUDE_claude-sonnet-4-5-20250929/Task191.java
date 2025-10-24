
public class Task191 {
    public static int longestValidParentheses(String s) {
        if (s == null || s.length() == 0) {
            return 0;
        }
        
        int maxLen = 0;
        int[] dp = new int[s.length()];
        
        for (int i = 1; i < s.length(); i++) {
            if (s.charAt(i) == ')') {
                if (s.charAt(i - 1) == '(') {
                    dp[i] = (i >= 2 ? dp[i - 2] : 0) + 2;
                } else if (i - dp[i - 1] > 0 && s.charAt(i - dp[i - 1] - 1) == '(') {
                    dp[i] = dp[i - 1] + 2 + (i - dp[i - 1] >= 2 ? dp[i - dp[i - 1] - 2] : 0);
                }
                maxLen = Math.max(maxLen, dp[i]);
            }
        }
        
        return maxLen;
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test 1: " + longestValidParentheses("(()"));
        
        // Test case 2
        System.out.println("Test 2: " + longestValidParentheses(")()())"));
        
        // Test case 3
        System.out.println("Test 3: " + longestValidParentheses(""));
        
        // Test case 4
        System.out.println("Test 4: " + longestValidParentheses("()(()"));
        
        // Test case 5
        System.out.println("Test 5: " + longestValidParentheses("(()())"));
    }
}
