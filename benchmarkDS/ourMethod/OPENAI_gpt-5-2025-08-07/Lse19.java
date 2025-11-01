import java.math.BigInteger;

public class Lse19 {
    private static final int MAX_INPUT_LENGTH = 1000;
    private static final BigInteger ONE_THOUSAND = BigInteger.valueOf(1000);

    public static BigInteger addThousand(BigInteger n) {
        if (n == null) {
            throw new IllegalArgumentException("Invalid input");
        }
        return n.add(ONE_THOUSAND);
    }

    private static boolean isValidIntegerString(String s, int maxLen) {
        if (s == null) return false;
        if (s.length() == 0 || s.length() > maxLen) return false;
        int start = 0;
        char c0 = s.charAt(0);
        if (c0 == '+' || c0 == '-') {
            if (s.length() == 1) return false;
            start = 1;
        }
        for (int i = start; i < s.length(); i++) {
            char ch = s.charAt(i);
            if (ch < '0' || ch > '9') return false;
        }
        return true;
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] tests = new String[] {
            "-1000",
            "0",
            "42",
            "2147483647",
            "9223372036854774807"
        };
        for (String t : tests) {
            if (isValidIntegerString(t, MAX_INPUT_LENGTH)) {
                BigInteger v = new BigInteger(t);
                System.out.println("addThousand(" + t + ") = " + addThousand(v));
            } else {
                System.out.println("Invalid test input: " + t);
            }
        }

        // Command-line processing
        if (args.length >= 1) {
            String input = args[0];
            if (!isValidIntegerString(input, MAX_INPUT_LENGTH)) {
                System.err.println("Invalid input");
                System.exit(1);
            }
            BigInteger value = new BigInteger(input);
            BigInteger result = addThousand(value);
            System.out.println(result.toString());
        }
    }
}