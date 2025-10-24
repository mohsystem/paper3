public class Task10 {

    /**
     * Given two integers a and b, which can be positive or negative,
     * find the sum of all the integers between and including them and return it.
     * If the two numbers are equal return a or b.
     *
     * @param a an integer.
     * @param b an integer.
     * @return the sum of all integers between a and b (inclusive).
     */
    public static long getSum(int a, int b) {
        // If the numbers are the same, return one of them.
        if (a == b) {
            return a;
        }

        // Use long to prevent potential integer overflow during calculation.
        long min = Math.min(a, b);
        long max = Math.max(a, b);
        
        // Apply the formula for the sum of an arithmetic series:
        // Sum = n/2 * (first + last)
        // where n is the number of terms (max - min + 1)
        return (max - min + 1) * (min + max) / 2;
    }

    public static void main(String[] args) {
        // Test cases
        System.out.println(getSum(1, 0));    // Expected: 1
        System.out.println(getSum(1, 2));    // Expected: 3
        System.out.println(getSum(1, 1));    // Expected: 1
        System.out.println(getSum(-1, 0));   // Expected: -1
        System.out.println(getSum(-1, 2));   // Expected: 2
    }
}