import java.math.BigInteger;

public class Lse21 {
    // Adds 1000 to the provided BigInteger value and returns the result
    public static BigInteger add1000(BigInteger value) {
        if (value == null) {
            throw new IllegalArgumentException("Input value cannot be null");
        }
        return value.add(BigInteger.valueOf(1000));
    }

    public static void main(String[] args) {
        // Process command line input: read a value, add 1000, print result
        if (args != null && args.length > 0) {
            try {
                BigInteger input = new BigInteger(args[0].trim());
                BigInteger result = add1000(input);
                System.out.println(result.toString());
            } catch (NumberFormatException ex) {
                System.err.println("Invalid integer input");
            } catch (Exception ex) {
                System.err.println("Error: " + ex.getMessage());
            }
        }

        // 5 test cases
        BigInteger[] tests = new BigInteger[] {
            BigInteger.ZERO,
            new BigInteger("12345"),
            new BigInteger("-1000"),
            new BigInteger(Long.toString(Long.MAX_VALUE)),
            new BigInteger("999999999999999999999999999999999999")
        };
        for (BigInteger t : tests) {
            try {
                BigInteger res = add1000(t);
                System.out.println(res.toString());
            } catch (Exception ex) {
                System.err.println("Test error: " + ex.getMessage());
            }
        }

        System.exit(0);
    }
}