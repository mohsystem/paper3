public class Task142 {

    /**
     * Calculates the greatest common divisor (GCD) of two numbers using the iterative Euclidean algorithm.
     * This method handles positive, negative, and zero inputs. The result is always non-negative.
     *
     * @param a The first number.
     * @param b The second number.
     * @return The greatest common divisor of a and b.
     */
    public static long gcd(long a, long b) {
        // The GCD is conventionally a non-negative number.
        // gcd(a, b) = gcd(|a|, |b|).
        a = Math.abs(a);
        b = Math.abs(b);

        while (b != 0) {
            long temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }

    public static void main(String[] args) {
        // 5 Test Cases
        long[][] testCases = {
            {54, 24},      // Standard case
            {101, 103},    // Coprime numbers
            {0, 5},        // One number is zero
            {12, 0},        // The other number is zero
            {-48, -18}     // Negative numbers
        };

        for (long[] testCase : testCases) {
            long num1 = testCase[0];
            long num2 = testCase[1];
            long result = gcd(num1, num2);
            System.out.println("GCD of " + num1 + " and " + num2 + " is: " + result);
        }
    }
}