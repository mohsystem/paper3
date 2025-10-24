// Task183 - Java implementation
// Chain-of-Through secure coding process:
// 1) Problem understanding: compute the largest product of any contiguous series of a given span in a digit string.
// 2) Security requirements: validate inputs (null, negative span, non-digits, span bounds), avoid unchecked operations.
// 3) Secure coding generation: use safe loops, early returns for invalid states, prevent division by zero (not used), handle span==0.
// 4) Code review: ensure no unchecked null dereference, index bounds respected, simple O(n*span) approach to avoid complexity issues.
// 5) Secure code output: final code returns consistent results and includes tests.

public class Task183 {
    public static long largestProduct(String digits, int span) {
        if (span < 0) return 0L;
        if (span == 0) return 1L;
        if (digits == null) return 0L;
        final int n = digits.length();
        if (span > n) return 0L;

        // Validate all characters are digits
        for (int i = 0; i < n; i++) {
            char c = digits.charAt(i);
            if (c < '0' || c > '9') {
                return 0L;
            }
        }

        long maxProduct = 0L;
        for (int i = 0; i <= n - span; i++) {
            long product = 1L;
            boolean zeroFound = false;
            for (int j = i; j < i + span; j++) {
                int d = digits.charAt(j) - '0';
                if (d == 0) {
                    zeroFound = true;
                    product = 0L;
                    break; // no need to continue this window
                }
                // Overflow-safe-ish check (best effort); for typical spans <= 19 this stays within long range.
                if (product > Long.MAX_VALUE / d) {
                    // Overflow would occur; cap to Long.MAX_VALUE for comparison purposes
                    product = Long.MAX_VALUE;
                    // No break to remain deterministic; but result is effectively max.
                } else {
                    product *= d;
                }
            }
            if (product > maxProduct) {
                maxProduct = product;
            }
            // Optimization: if max is Long.MAX_VALUE, cannot get larger
            if (maxProduct == Long.MAX_VALUE) break;
        }
        return maxProduct;
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] inputs = {
            "63915",
            "1234567890",
            "0000",
            "99999",
            ""
        };
        int[] spans = {
            3,
            2,
            2,
            5,
            0
        };
        for (int i = 0; i < inputs.length; i++) {
            long result = largestProduct(inputs[i], spans[i]);
            System.out.println("Test " + (i + 1) + ": largestProduct(\"" + inputs[i] + "\", " + spans[i] + ") = " + result);
        }
    }
}