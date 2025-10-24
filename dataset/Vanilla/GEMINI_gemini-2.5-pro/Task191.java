class Task191 {
    public int longestValidParentheses(String s) {
        if (s == null || s.length() == 0) {
            return 0;
        }
        int left = 0, right = 0, maxLength = 0;

        // Left to Right scan
        for (int i = 0; i < s.length(); i++) {
            if (s.charAt(i) == '(') {
                left++;
            } else {
                right++;
            }
            if (left == right) {
                maxLength = Math.max(maxLength, 2 * right);
            } else if (right > left) {
                left = 0;
                right = 0;
            }
        }

        left = 0;
        right = 0;

        // Right to Left scan
        for (int i = s.length() - 1; i >= 0; i--) {
            if (s.charAt(i) == '(') {
                left++;
            } else {
                right++;
            }
            if (left == right) {
                maxLength = Math.max(maxLength, 2 * left);
            } else if (left > right) {
                left = 0;
                right = 0;
            }
        }
        return maxLength;
    }

    public static void main(String[] args) {
        Task191 solution = new Task191();
        
        // Test Case 1
        String s1 = "(()";
        System.out.println("Input: " + s1 + ", Output: " + solution.longestValidParentheses(s1));

        // Test Case 2
        String s2 = ")()())";
        System.out.println("Input: " + s2 + ", Output: " + solution.longestValidParentheses(s2));

        // Test Case 3
        String s3 = "";
        System.out.println("Input: " + s3 + ", Output: " + solution.longestValidParentheses(s3));

        // Test Case 4
        String s4 = "()(()";
        System.out.println("Input: " + s4 + ", Output: " + solution.longestValidParentheses(s4));

        // Test Case 5
        String s5 = "()(())";
        System.out.println("Input: " + s5 + ", Output: " + solution.longestValidParentheses(s5));
    }
}