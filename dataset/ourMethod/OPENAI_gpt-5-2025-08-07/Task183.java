import java.math.BigInteger;

public class Task183 {

    public static final class ProductResult {
        public final boolean ok;
        public final BigInteger value;

        private ProductResult(boolean ok, BigInteger value) {
            this.ok = ok;
            this.value = value;
        }

        public static ProductResult ok(BigInteger v) {
            return new ProductResult(true, v);
        }

        public static ProductResult err() {
            return new ProductResult(false, BigInteger.valueOf(-1));
        }
    }

    private static final int MAX_LEN = 1_000_000;

    public static ProductResult largestProduct(String digits, int span) {
        if (digits == null) return ProductResult.err();
        final int n = digits.length();
        if (n > MAX_LEN) return ProductResult.err();
        if (span < 0 || span > n) return ProductResult.err();

        for (int i = 0; i < n; i++) {
            char c = digits.charAt(i);
            if (c < '0' || c > '9') return ProductResult.err();
        }

        if (span == 0) return ProductResult.ok(BigInteger.ONE);
        if (span == 1) {
            int maxDigit = 0;
            for (int i = 0; i < n; i++) {
                int d = digits.charAt(i) - '0';
                if (d > maxDigit) maxDigit = d;
            }
            return ProductResult.ok(BigInteger.valueOf(maxDigit));
        }

        BigInteger max = BigInteger.ZERO;
        for (int i = 0; i + span <= n; i++) {
            BigInteger prod = BigInteger.ONE;
            for (int j = i; j < i + span; j++) {
                int d = digits.charAt(j) - '0';
                prod = prod.multiply(BigInteger.valueOf(d));
                if (prod.signum() == 0) break;
            }
            if (prod.compareTo(max) > 0) {
                max = prod;
            }
        }
        return ProductResult.ok(max);
    }

    private static void runTest(String digits, int span) {
        ProductResult r = largestProduct(digits, span);
        if (r.ok) {
            System.out.println(r.value.toString());
        } else {
            System.out.println("ERROR");
        }
    }

    public static void main(String[] args) {
        // Test 1: Example
        runTest("63915", 3);

        // Test 2: Contains zeros
        runTest("10203", 2);

        // Test 3: Long sequence with span 6
        String longSeq = "73167176531330624919225119674426574742355349194934";
        runTest(longSeq, 6);

        // Test 4: Span 0
        runTest("12345", 0);

        // Test 5: 25 nines, span 21 (large product, handled by BigInteger)
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 25; i++) sb.append('9');
        runTest(sb.toString(), 21);
    }
}