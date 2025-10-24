import java.util.Arrays;

public class Task57 {

    public static final class FactorialResult {
        public final boolean success;
        public final long value;
        public final String error;

        public FactorialResult(boolean success, long value, String error) {
            this.success = success;
            this.value = value;
            this.error = error;
        }
    }

    public static FactorialResult factorial(int n) {
        if (n < 0) {
            return new FactorialResult(false, 0L, "Input must be a non-negative integer.");
        }
        // Using 64-bit signed long; 20! fits, 21! overflows
        if (n > 20) {
            return new FactorialResult(false, 0L, "Input too large; maximum supported is 20 for 64-bit.");
        }
        long acc = 1L;
        for (int i = 2; i <= n; i++) {
            acc *= i;
        }
        return new FactorialResult(true, acc, "");
    }

    public static void main(String[] args) {
        int[] tests = new int[] {0, 1, 5, 20, 21};
        for (int n : tests) {
            FactorialResult res = factorial(n);
            if (res.success) {
                System.out.println("factorial(" + n + ") = " + res.value);
            } else {
                System.out.println("factorial(" + n + ") error: " + res.error);
            }
        }
    }
}