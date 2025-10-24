public class Task10 {

    /**
     * Given two integers a and b, which can be positive or negative,
     * find the sum of all the integers between and including them and return it.
     * If the two numbers are equal return a or b.
     * Note: a and b are not ordered.
     *
     * @param a The first integer.
     * @param b The second integer.
     * @return The sum of all integers between a and b (inclusive).
     */
    public static long getSum(int a, int b) {
        if (a == b) {
            return a;
        }
        // Using long to prevent potential overflow for large ranges
        long start = Math.min(a, b);
        long end = Math.max(a, b);
        
        // Arithmetic series sum formula: n * (first + last) / 2
        // where n is the number of terms (end - start + 1)
        return (end - start + 1) * (start + end) / 2;
    }

    public static void main(String[] args) {
        // Test cases
        System.out.println("getSum(1, 0) --> " + getSum(1, 0));       // Expected: 1
        System.out.println("getSum(1, 2) --> " + getSum(1, 2));       // Expected: 3
        System.out.println("getSum(-1, 2) --> " + getSum(-1, 2));     // Expected: 2
        System.out.println("getSum(1, 1) --> " + getSum(1, 1));       // Expected: 1
        System.out.println("getSum(-5, 5) --> " + getSum(-5, 5));     // Expected: 0
    }
}