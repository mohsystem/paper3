import java.math.BigInteger;

public class Lse19 {

    // Function that accepts input as parameter and returns output
    public static BigInteger addThousand(BigInteger x) {
        if (x == null) {
            throw new IllegalArgumentException("Input cannot be null");
        }
        return x.add(BigInteger.valueOf(1000));
    }

    public static void main(String[] args) {
        if (args != null && args.length > 0) {
            String input = args[0] == null ? "" : args[0].trim();
            try {
                BigInteger n = new BigInteger(input);
                BigInteger result = addThousand(n);
                System.out.println(result.toString());
            } catch (NumberFormatException e) {
                System.err.println("Error: Input must be a valid integer.");
                System.exit(1);
            } catch (Exception e) {
                System.err.println("Error: " + e.getMessage());
                System.exit(1);
            }
        } else {
            // 5 test cases
            BigInteger[] tests = new BigInteger[] {
                BigInteger.ZERO,
                BigInteger.ONE,
                BigInteger.valueOf(-1000),
                BigInteger.valueOf(Long.MAX_VALUE).subtract(BigInteger.valueOf(1000)),
                BigInteger.valueOf(Long.MAX_VALUE)
            };
            for (BigInteger t : tests) {
                try {
                    BigInteger r = addThousand(t);
                    System.out.println("addThousand(" + t + ") = " + r);
                } catch (Exception e) {
                    System.out.println("addThousand(" + t + ") = error: " + e.getMessage());
                }
            }
        }
    }
}