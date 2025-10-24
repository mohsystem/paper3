import java.lang.Math;

public class Task34 {

    /**
     * Checks if a given number is a Narcissistic Number (Armstrong Number).
     * A number is narcissistic if it is the sum of its own digits, each raised to
     * the power of the number of digits.
     *
     * @param number The positive integer to check.
     * @return true if the number is narcissistic, false otherwise.
     */
    public static boolean isNarcissistic(int number) {
        String numberStr = String.valueOf(number);
        int numDigits = numberStr.length();
        long sum = 0;

        int temp = number;
        while (temp > 0) {
            int digit = temp % 10;
            sum += Math.pow(digit, numDigits);
            temp /= 10;
        }

        return sum == number;
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println("isNarcissistic(153): " + isNarcissistic(153));   // Expected: true
        System.out.println("isNarcissistic(1652): " + isNarcissistic(1652)); // Expected: false
        System.out.println("isNarcissistic(7): " + isNarcissistic(7));      // Expected: true
        System.out.println("isNarcissistic(371): " + isNarcissistic(371));   // Expected: true
        System.out.println("isNarcissistic(123): " + isNarcissistic(123));   // Expected: false
    }
}