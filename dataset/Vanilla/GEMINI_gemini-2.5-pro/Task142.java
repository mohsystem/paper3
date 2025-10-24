public class Task142 {

    /**
     * Finds the greatest common divisor (GCD) of two numbers using the Euclidean algorithm.
     *
     * @param a The first number.
     * @param b The second number.
     * @return The greatest common divisor of a and b.
     */
    public static int gcd(int a, int b) {
        if (b == 0) {
            return a;
        }
        return gcd(b, a % b);
    }

    public static void main(String[] args) {
        // Test cases
        System.out.println("Java Test Cases:");
        System.out.println("GCD(48, 18) = " + gcd(48, 18));         // Expected: 6
        System.out.println("GCD(101, 103) = " + gcd(101, 103));   // Expected: 1
        System.out.println("GCD(56, 98) = " + gcd(56, 98));       // Expected: 14
        System.out.println("GCD(270, 192) = " + gcd(270, 192));   // Expected: 6
        System.out.println("GCD(15, 0) = " + gcd(15, 0));         // Expected: 15
    }
}