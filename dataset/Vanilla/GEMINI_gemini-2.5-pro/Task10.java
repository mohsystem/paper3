public class Task10 {

    /**
     * Given two integers a and b, which can be positive or negative,
     * find the sum of all the integers between and including them and return it.
     * If the two numbers are equal return a or b.
     *
     * @param a The first integer.
     * @param b The second integer.
     * @return The sum of all integers between and including a and b.
     */
    public static int getSum(int a, int b) {
        // Use the formula for the sum of an arithmetic series:
        // sum = (number of terms / 2) * (first term + last term)
        // number of terms = abs(a - b) + 1
        // first term + last term = a + b
        // The formula correctly handles the case where a == b:
        // sum = ( (0 + 1) / 2.0 ) * (a + a) = 0.5 * 2a = a
        // We use long for the intermediate multiplication to prevent integer overflow.
        return (int) ((long)(a + b) * (Math.abs(a - b) + 1) / 2);
    }

    public static void main(String[] args) {
        // Test cases
        System.out.println(getSum(1, 0));   // Expected: 1
        System.out.println(getSum(1, 2));   // Expected: 3
        System.out.println(getSum(1, 1));   // Expected: 1
        System.out.println(getSum(-1, 0));  // Expected: -1
        System.out.println(getSum(-1, 2));  // Expected: 2
    }
}