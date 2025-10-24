public class Task2 {

    /**
     * Accepts an array of 10 integers (between 0 and 9) and returns a string
     * of those numbers in the form of a phone number.
     *
     * @param numbers An array of 10 integers (0-9).
     * @return A formatted phone number string, e.g., "(123) 456-7890".
     */
    public static String createPhoneNumber(int[] numbers) {
        // Input validation to ensure array is not null and has the correct length.
        if (numbers == null || numbers.length != 10) {
            throw new IllegalArgumentException("Input array must contain exactly 10 integers.");
        }
        
        // Use String.format for a safe and readable way to create the formatted string.
        // This avoids manual concatenation which can be less efficient and harder to read.
        return String.format("(%d%d%d) %d%d%d-%d%d%d%d",
                numbers[0], numbers[1], numbers[2],
                numbers[3], numbers[4], numbers[5],
                numbers[6], numbers[7], numbers[8], numbers[9]);
    }

    public static void main(String[] args) {
        // 5 Test Cases
        int[][] testCases = {
            {1, 2, 3, 4, 5, 6, 7, 8, 9, 0},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {0, 9, 8, 7, 6, 5, 4, 3, 2, 1},
            {5, 5, 5, 8, 6, 7, 5, 3, 0, 9},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        };

        for (int i = 0; i < testCases.length; i++) {
            try {
                String phoneNumber = createPhoneNumber(testCases[i]);
                System.out.println("Test Case " + (i + 1) + ": " + phoneNumber);
            } catch (IllegalArgumentException e) {
                System.out.println("Test Case " + (i + 1) + ": Error - " + e.getMessage());
            }
        }
    }
}