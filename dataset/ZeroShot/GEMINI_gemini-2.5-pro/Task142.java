public class Task142 {

    /**
     * Calculates the greatest common divisor (GCD) of two numbers using the iterative Euclidean algorithm.
     * This method is secure against stack overflow by being iterative and handles edge cases
     * such as zero and negative inputs by taking their absolute values.
     *
     * @param a The first number. Using long to handle a wider range of values.
     * @param b The second number. Using long to handle a wider range of values.
     * @return The greatest common divisor of a and b.
     */
    public static long gcd(long a, long b) {
        // The GCD is conventionally defined for positive integers.
        // gcd(a, b) is equal to gcd(|a|, |b|).
        a = Math.abs(a);
        b = Math.abs(b);

        // Iterative Euclidean algorithm
        while (b != 0) {
            long temp = b;
            b = a % b;
            a = temp;
        }

        // The algorithm naturally handles cases like gcd(n, 0) = n and gcd(0, 0) = 0.
        return a;
    }

    public static void main(String[] args) {
        System.out.println("Running test cases for Java GCD:");

        // Test Case 1: Standard case
        long a1 = 48, b1 = 18;
        System.out.println("GCD(" + a1 + ", " + b1 + ") = " + gcd(a1, b1));

        // Test Case 2: Coprime numbers (GCD is 1)
        long a2 = 101, b2 = 103;
        System.out.println("GCD(" + a2 + ", " + b2 + ") = " + gcd(a2, b2));

        // Test Case 3: One number is zero
        long a3 = 56, b3 = 0;
        System.out.println("GCD(" + a3 + ", " + b3 + ") = " + gcd(a3, b3));

        // Test Case 4: Negative inputs
        long a4 = -60, b4 = 48;
        System.out.println("GCD(" + a4 + ", " + b4 + ") = " + gcd(a4, b4));

        // Test Case 5: Both numbers are zero
        long a5 = 0, b5 = 0;
        System.out.println("GCD(" + a5 + ", " + b5 + ") = " + gcd(a5, b5));
    }
}