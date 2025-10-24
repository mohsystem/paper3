public class Task5 {

    /**
     * Calculates the multiplicative persistence of a positive number.
     * Multiplicative persistence is the number of times you must multiply the digits
     * in a number until you reach a single digit.
     *
     * @param num A non-negative long integer.
     * @return The multiplicative persistence of num.
     * @throws IllegalArgumentException if the input number is negative.
     */
    public static int persistence(long num) {
        // Secure coding: Validate input to ensure it's non-negative as per the problem's implicit constraints.
        if (num < 0) {
            throw new IllegalArgumentException("Input must be a non-negative number.");
        }

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
        // 5 Test Cases
        System.out.println("persistence(39) -> " + persistence(39));      // Expected: 3
        System.out.println("persistence(999) -> " + persistence(999));    // Expected: 4
        System.out.println("persistence(4) -> " + persistence(4));        // Expected: 0
        System.out.println("persistence(25) -> " + persistence(25));      // Expected: 2 (2*5=10, 1*0=0)
        System.out.println("persistence(679) -> " + persistence(679));    // Expected: 5 (6*7*9=378, 3*7*8=168, 1*6*8=48, 4*8=32, 3*2=6)
    }
}