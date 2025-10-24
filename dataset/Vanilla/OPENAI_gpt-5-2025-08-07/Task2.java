public class Task2 {
    public static String createPhoneNumber(int[] numbers) {
        if (numbers == null || numbers.length != 10) {
            throw new IllegalArgumentException("Input must be 10 integers");
        }
        for (int n : numbers) {
            if (n < 0 || n > 9) {
                throw new IllegalArgumentException("Digits must be between 0 and 9");
            }
        }
        return String.format("(%d%d%d) %d%d%d-%d%d%d%d",
                numbers[0], numbers[1], numbers[2],
                numbers[3], numbers[4], numbers[5],
                numbers[6], numbers[7], numbers[8], numbers[9]);
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
                {1,2,3,4,5,6,7,8,9,0},
                {0,0,0,0,0,0,0,0,0,0},
                {9,8,7,6,5,4,3,2,1,0},
                {5,5,5,1,2,3,4,5,6,7},
                {1,2,3,1,2,3,1,2,3,4}
        };
        for (int i = 0; i < tests.length; i++) {
            System.out.println(createPhoneNumber(tests[i]));
        }
    }
}