public class Task5 {

    /**
     * Calculates the multiplicative persistence of a positive number.
     *
     * @param num A positive long integer.
     * @return The number of times digits must be multiplied to reach a single digit.
     */
    public static int persistence(long num) {
        int count = 0;
        while (num >= 10) {
            count++;
            long product = 1;
            long temp = num;
            while (temp > 0) {
                product *= temp % 10;
                temp /= 10;
            }
            num = product;
        }
        return count;
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println("Input: 39, Output: " + persistence(39));      // Expected: 3
        System.out.println("Input: 999, Output: " + persistence(999));    // Expected: 4
        System.out.println("Input: 4, Output: " + persistence(4));        // Expected: 0
        System.out.println("Input: 25, Output: " + persistence(25));      // Expected: 2 (2*5=10, 1*0=0)
        System.out.println("Input: 679, Output: " + persistence(679));    // Expected: 5 (6*7*9=378, 3*7*8=168, 1*6*8=48, 4*8=32, 3*2=6)
    }
}