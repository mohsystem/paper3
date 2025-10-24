
import java.util.Arrays;

public class Task2 {
    public static String createPhoneNumber(int[] numbers) {
        // Validate input is not null
        if (numbers == null) {
            throw new IllegalArgumentException("Input array cannot be null");
        }
        
        // Validate array length
        if (numbers.length != 10) {
            throw new IllegalArgumentException("Input array must contain exactly 10 elements");
        }
        
        // Validate each element is between 0 and 9
        for (int i = 0; i < numbers.length; i++) {
            if (numbers[i] < 0 || numbers[i] > 9) {
                throw new IllegalArgumentException("All elements must be between 0 and 9");
            }
        }
        
        // Format phone number using constant format string
        return String.format("(%d%d%d) %d%d%d-%d%d%d%d",
            numbers[0], numbers[1], numbers[2],
            numbers[3], numbers[4], numbers[5],
            numbers[6], numbers[7], numbers[8], numbers[9]);
    }
    
    public static void main(String[] args) {
        // Test case 1: Standard example
        int[] test1 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
        System.out.println(createPhoneNumber(test1));
        
        // Test case 2: All zeros
        int[] test2 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        System.out.println(createPhoneNumber(test2));
        
        // Test case 3: All nines
        int[] test3 = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
        System.out.println(createPhoneNumber(test3));
        
        // Test case 4: Mixed digits
        int[] test4 = {5, 5, 5, 1, 2, 3, 4, 5, 6, 7};
        System.out.println(createPhoneNumber(test4));
        
        // Test case 5: Another pattern
        int[] test5 = {8, 6, 7, 5, 3, 0, 9, 1, 2, 3};
        System.out.println(createPhoneNumber(test5));
    }
}
