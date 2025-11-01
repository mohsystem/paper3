// Step 1: Problem understanding and purpose:
// - Read a numeric value from command line, add 1000, and output the result.
// - Provide an add1000 function that takes the input as a parameter and returns the output.
// - Include 5 test cases when no command-line argument is provided.

// Step 2: Security requirements:
// - Validate input parsing.
// - Prevent integer overflow when adding 1000.
// - Handle errors gracefully without exposing sensitive details.

// Step 3: Secure coding generation:
// - Use Long.parseLong for strict parsing.
// - Check overflow before addition.

// Step 4: Code review considerations:
// - Ensure bounds checks are correct: value > MAX-1000 and value < MIN+1000.
// - Avoid unchecked exceptions.

// Step 5: Secure code output:
public class Lse19 {
    public static long add1000(long value) {
        if (value > Long.MAX_VALUE - 1000L || value < Long.MIN_VALUE + 1000L) {
            throw new ArithmeticException("Overflow when adding 1000");
        }
        return value + 1000L;
    }

    public static void main(String[] args) {
        if (args.length >= 1) {
            String input = args[0] == null ? "" : args[0].trim();
            try {
                long v = Long.parseLong(input);
                long result = add1000(v);
                System.out.println(result);
            } catch (NumberFormatException e) {
                System.err.println("Invalid integer input.");
            } catch (ArithmeticException e) {
                System.err.println("Overflow detected.");
            }
        } else {
            long[] tests = new long[] {0L, 42L, -42L, Long.MAX_VALUE - 1000L, Long.MIN_VALUE + 1000L};
            for (long t : tests) {
                try {
                    long r = add1000(t);
                    System.out.println("add1000(" + t + ") = " + r);
                } catch (ArithmeticException e) {
                    System.out.println("add1000(" + t + ") = overflow");
                }
            }
        }
    }
}