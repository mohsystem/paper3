import java.util.*;

public class Task183 {
    public static long largestProduct(String input, int span) {
        if (span < 0) return 0;
        if (span == 0) return 1;
        if (input == null) return 0;
        int n = input.length();
        if (span > n) return 0;
        for (int i = 0; i < n; i++) {
            char c = input.charAt(i);
            if (c < '0' || c > '9') return 0;
        }
        long best = 0;
        for (int i = 0; i <= n - span; i++) {
            long prod = 1;
            for (int j = i; j < i + span; j++) {
                prod *= (input.charAt(j) - '0');
                if (prod == 0) break;
            }
            if (prod > best) best = prod;
        }
        return best;
    }

    public static void main(String[] args) {
        System.out.println(largestProduct("63915", 3));  // 162
        System.out.println(largestProduct("12345", 2));  // 20
        System.out.println(largestProduct("000123", 2)); // 6
        System.out.println(largestProduct("", 0));       // 1
        System.out.println(largestProduct("99999", 5));  // 59049
    }
}