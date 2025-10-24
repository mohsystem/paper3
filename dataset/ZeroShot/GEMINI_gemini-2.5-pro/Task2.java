public class Task2 {

    /**
     * Formats an array of 10 integers (0-9) into a phone number string.
     *
     * @param numbers The array of 10 integers.
     * @return A formatted string like "(123) 456-7890".
     * @throws IllegalArgumentException if the input is invalid.
     */
    public static String createPhoneNumber(int[] numbers) {
        // Security: Validate input to prevent unexpected errors.
        if (numbers == null || numbers.length != 10) {
            throw new IllegalArgumentException("Input array must contain exactly 10 integers.");
        }

        StringBuilder phoneNumber = new StringBuilder(14);
        phoneNumber.append("(");
        for (int i = 0; i < 10; i++) {
            int num = numbers[i];
            // Security: Ensure all numbers are single digits (0-9).
            if (num < 0 || num > 9) {
                throw new IllegalArgumentException("All numbers in the array must be between 0 and 9.");
            }
            phoneNumber.append(num);
            if (i == 2) {
                phoneNumber.append(") ");
            } else if (i == 5) {
                phoneNumber.append("-");
            }
        }
        return phoneNumber.toString();
    }

    public static void main(String[] args) {
        // Test Case 1: Valid input
        int[] case1 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
        System.out.println("Test Case 1: " + createPhoneNumber(case1));

        // Test Case 2: Valid input with zeros
        int[] case2 = {0, 0, 0, 1, 1, 1, 2, 2, 2, 2};
        System.out.println("Test Case 2: " + createPhoneNumber(case2));

        // Test Case 3: Invalid length (too short)
        try {
            int[] case3 = {1, 2, 3};
            System.out.println("Test Case 3: " + createPhoneNumber(case3));
        } catch (IllegalArgumentException e) {
            System.out.println("Test Case 3: Caught expected exception - " + e.getMessage());
        }

        // Test Case 4: Invalid number (out of range)
        try {
            int[] case4 = {1, 2, 3, 4, 15, 6, 7, 8, 9, 0};
            System.out.println("Test Case 4: " + createPhoneNumber(case4));
        } catch (IllegalArgumentException e) {
            System.out.println("Test Case 4: Caught expected exception - " + e.getMessage());
        }

        // Test Case 5: Invalid input (null)
        try {
            System.out.println("Test Case 5: " + createPhoneNumber(null));
        } catch (IllegalArgumentException e) {
            System.out.println("Test Case 5: Caught expected exception - " + e.getMessage());
        }
    }
}