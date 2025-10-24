public class Task34 {

    /**
     * Checks if a given number is a Narcissistic Number (Armstrong Number).
     *
     * @param number The positive integer to check.
     * @return true if the number is narcissistic, false otherwise.
     */
    public static boolean isNarcissistic(int number) {
        if (number <= 0) {
            return false;
        }

        String numberStr = String.valueOf(number);
        int digits = numberStr.length();
        long sum = 0;
        int temp = number;

        while (temp > 0) {
            int digit = temp % 10;
            sum += Math.pow(digit, digits);
            temp /= 10;
        }

        return sum == number;
    }

    public static void main(String[] args) {
        int[] testCases = {153, 1652, 1, 371, 9474};
        for (int testCase : testCases) {
            System.out.println(isNarcissistic(testCase));
        }
    }
}