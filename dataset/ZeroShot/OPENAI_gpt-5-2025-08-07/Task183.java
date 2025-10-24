import java.math.BigInteger;
import java.util.Arrays;

public class Task183 {
    public static BigInteger largestProduct(String input, int span) {
        if (input == null) {
            throw new IllegalArgumentException("Input cannot be null.");
        }
        if (span < 0) {
            throw new IllegalArgumentException("Span cannot be negative.");
        }
        if (span == 0) {
            return BigInteger.ONE;
        }
        if (span > input.length()) {
            throw new IllegalArgumentException("Span cannot exceed input length.");
        }
        for (int i = 0; i < input.length(); i++) {
            char c = input.charAt(i);
            if (c < '0' || c > '9') {
                throw new IllegalArgumentException("Input must contain only digits.");
            }
        }
        int n = input.length();
        int[] digits = new int[n];
        for (int i = 0; i < n; i++) {
            digits[i] = input.charAt(i) - '0';
        }

        int zeros = 0;
        BigInteger product = BigInteger.ONE;
        boolean productValid = true;

        for (int i = 0; i < span; i++) {
            int d = digits[i];
            if (d == 0) {
                zeros++;
                productValid = false;
            } else {
                product = product.multiply(BigInteger.valueOf(d));
            }
        }

        BigInteger max = zeros == 0 ? product : BigInteger.ZERO;

        for (int i = span; i < n; i++) {
            int outgoing = digits[i - span];
            int incoming = digits[i];

            if (outgoing == 0) {
                zeros--;
                productValid = false;
            } else if (zeros == 0 && productValid) {
                product = product.divide(BigInteger.valueOf(outgoing));
            }

            if (incoming == 0) {
                zeros++;
                productValid = false;
            } else if (zeros == 0 && productValid) {
                product = product.multiply(BigInteger.valueOf(incoming));
            }

            if (zeros == 0 && !productValid) {
                product = BigInteger.ONE;
                for (int k = i - span + 1; k <= i; k++) {
                    product = product.multiply(BigInteger.valueOf(digits[k]));
                }
                productValid = true;
            }

            if (zeros == 0 && product.compareTo(max) > 0) {
                max = product;
            }
        }

        return max;
    }

    public static void main(String[] args) {
        String[] inputs = { "63915", "123456789", "1020304050", "000", "" };
        int[] spans = { 3, 2, 2, 2, 0 };

        for (int i = 0; i < inputs.length; i++) {
            try {
                BigInteger result = largestProduct(inputs[i], spans[i]);
                System.out.println("Input: " + inputs[i] + ", Span: " + spans[i] + " -> " + result.toString());
            } catch (IllegalArgumentException e) {
                System.out.println("Input: " + inputs[i] + ", Span: " + spans[i] + " -> Error: " + e.getMessage());
            }
        }
    }
}