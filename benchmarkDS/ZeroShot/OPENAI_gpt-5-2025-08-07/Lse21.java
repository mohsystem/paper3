import java.math.BigInteger;

public class Lse21 {
    public static BigInteger addThousand(BigInteger x) {
        if (x == null) {
            throw new IllegalArgumentException("Input cannot be null");
        }
        return x.add(BigInteger.valueOf(1000));
    }

    public static void main(String[] args) {
        try {
            if (args != null && args.length > 0) {
                String s = args[0] == null ? "" : args[0].trim();
                BigInteger value = new BigInteger(s);
                BigInteger result = addThousand(value);
                System.out.println(result.toString());
            } else {
                BigInteger[] tests = new BigInteger[]{
                    new BigInteger("-1000"),
                    new BigInteger("-1"),
                    BigInteger.ZERO,
                    BigInteger.ONE,
                    new BigInteger("9223372036854774800")
                };
                for (BigInteger t : tests) {
                    System.out.println(addThousand(t));
                }
            }
        } catch (NumberFormatException e) {
            System.err.println("Invalid integer input.");
        } catch (Exception e) {
            System.err.println("An unexpected error occurred.");
        } finally {
            System.exit(0);
        }
    }
}