public class Task2 {

    public static String createPhoneNumber(int[] numbers) {
        return String.format("(%d%d%d) %d%d%d-%d%d%d%d",
                numbers[0], numbers[1], numbers[2],
                numbers[3], numbers[4], numbers[5],
                numbers[6], numbers[7], numbers[8], numbers[9]);
    }

    public static void main(String[] args) {
        // Test cases
        int[][] testCases = {
            {1, 2, 3, 4, 5, 6, 7, 8, 9, 0},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {8, 0, 0, 5, 5, 5, 1, 2, 3, 4},
            {0, 9, 8, 7, 6, 5, 4, 3, 2, 1},
            {5, 1, 9, 5, 5, 5, 4, 4, 6, 8}
        };

        for (int[] testCase : testCases) {
            System.out.println(createPhoneNumber(testCase));
        }
    }
}