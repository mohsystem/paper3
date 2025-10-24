import java.util.Arrays;

public class Task2 {

    public static String createPhoneNumber(int[] numbers) {
        if (numbers == null) {
            throw new IllegalArgumentException("Input array must not be null");
        }
        if (numbers.length != 10) {
            throw new IllegalArgumentException("Input array must have exactly 10 digits");
        }
        StringBuilder sb = new StringBuilder(14);
        sb.append('(');
        for (int i = 0; i < 3; i++) {
            int d = numbers[i];
            if (d < 0 || d > 9) {
                throw new IllegalArgumentException("Each element must be in range 0..9 at index " + i);
            }
            sb.append((char) ('0' + d));
        }
        sb.append(") ");
        for (int i = 3; i < 6; i++) {
            int d = numbers[i];
            if (d < 0 || d > 9) {
                throw new IllegalArgumentException("Each element must be in range 0..9 at index " + i);
            }
            sb.append((char) ('0' + d));
        }
        sb.append('-');
        for (int i = 6; i < 10; i++) {
            int d = numbers[i];
            if (d < 0 || d > 9) {
                throw new IllegalArgumentException("Each element must be in range 0..9 at index " + i);
            }
            sb.append((char) ('0' + d));
        }
        return sb.toString();
    }

    private static void runTest(int[] arr) {
        try {
            String result = createPhoneNumber(arr);
            System.out.println(Arrays.toString(arr) + " -> " + result);
        } catch (IllegalArgumentException ex) {
            System.out.println(Arrays.toString(arr) + " -> ERROR: " + ex.getMessage());
        }
    }

    public static void main(String[] args) {
        runTest(new int[]{1,2,3,4,5,6,7,8,9,0});      // valid
        runTest(new int[]{0,0,0,0,0,0,0,0,0,0});      // valid
        runTest(new int[]{9,7,6,5,4,3,2,1,0,9});      // valid
        runTest(new int[]{1,2,3,4,5,6,7,8,9});        // invalid length
        runTest(new int[]{1,2,3,4,5,6,7,8,9,10});     // invalid digit
    }
}