import java.util.Arrays;

public class Task2 {

    public static String createPhoneNumber(int[] numbers) {
        if (numbers == null) {
            throw new IllegalArgumentException("Input array cannot be null.");
        }
        if (numbers.length != 10) {
            throw new IllegalArgumentException("Input array must contain exactly 10 integers.");
        }
        for (int n : numbers) {
            if (n < 0 || n > 9) {
                throw new IllegalArgumentException("All numbers must be in the range 0 to 9.");
            }
        }
        StringBuilder sb = new StringBuilder(14);
        sb.append('(')
          .append(numbers[0]).append(numbers[1]).append(numbers[2])
          .append(") ")
          .append(numbers[3]).append(numbers[4]).append(numbers[5])
          .append('-')
          .append(numbers[6]).append(numbers[7]).append(numbers[8]).append(numbers[9]);
        return sb.toString();
    }

    public static void main(String[] args) {
        int[][] tests = new int[][]{
            new int[]{1,2,3,4,5,6,7,8,9,0},
            new int[]{0,0,0,0,0,0,0,0,0,0},
            new int[]{9,8,7,6,5,4,3,2,1,0},
            new int[]{1,1,1,1,1,1,1,1,1,1},
            new int[]{1,2,3,4,-5,6,7,8,9,0} // invalid
        };

        for (int i = 0; i < tests.length; i++) {
            try {
                String result = createPhoneNumber(tests[i]);
                System.out.println("Test " + (i+1) + " (" + Arrays.toString(tests[i]) + "): " + result);
            } catch (IllegalArgumentException ex) {
                System.out.println("Test " + (i+1) + " (" + Arrays.toString(tests[i]) + "): ERROR - " + ex.getMessage());
            }
        }
    }
}