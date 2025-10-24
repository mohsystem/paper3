
public class Task2 {
    public static String createPhoneNumber(int[] numbers) {
        if (numbers == null || numbers.length != 10) {
            throw new IllegalArgumentException("Array must contain exactly 10 integers");
        }
        
        for (int num : numbers) {
            if (num < 0 || num > 9) {
                throw new IllegalArgumentException("All numbers must be between 0 and 9");
            }
        }
        
        return String.format("(%d%d%d) %d%d%d-%d%d%d%d",
            numbers[0], numbers[1], numbers[2],
            numbers[3], numbers[4], numbers[5],
            numbers[6], numbers[7], numbers[8], numbers[9]);
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] test1 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
        System.out.println(createPhoneNumber(test1));
        
        // Test case 2
        int[] test2 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        System.out.println(createPhoneNumber(test2));
        
        // Test case 3
        int[] test3 = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
        System.out.println(createPhoneNumber(test3));
        
        // Test case 4
        int[] test4 = {5, 5, 5, 1, 2, 3, 4, 5, 6, 7};
        System.out.println(createPhoneNumber(test4));
        
        // Test case 5
        int[] test5 = {1, 1, 1, 2, 2, 2, 3, 3, 3, 4};
        System.out.println(createPhoneNumber(test5));
    }
}
