
public class Task2 {
    public static String createPhoneNumber(int[] numbers) {
        // Input validation
        if (numbers == null) {
            throw new IllegalArgumentException("Input array cannot be null");
        }
        
        if (numbers.length != 10) {
            throw new IllegalArgumentException("Array must contain exactly 10 elements");
        }
        
        // Validate each element is between 0-9
        for (int i = 0; i < numbers.length; i++) {
            if (numbers[i] < 0 || numbers[i] > 9) {
                throw new IllegalArgumentException("All elements must be between 0 and 9");
            }
        }
        
        // Build phone number using StringBuilder for security and efficiency
        StringBuilder phoneNumber = new StringBuilder();
        phoneNumber.append("(");
        phoneNumber.append(numbers[0]);
        phoneNumber.append(numbers[1]);
        phoneNumber.append(numbers[2]);
        phoneNumber.append(") ");
        phoneNumber.append(numbers[3]);
        phoneNumber.append(numbers[4]);
        phoneNumber.append(numbers[5]);
        phoneNumber.append("-");
        phoneNumber.append(numbers[6]);
        phoneNumber.append(numbers[7]);
        phoneNumber.append(numbers[8]);
        phoneNumber.append(numbers[9]);
        
        return phoneNumber.toString();
    }
    
    public static void main(String[] args) {
        // Test case 1: Standard input
        int[] test1 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
        System.out.println("Test 1: " + createPhoneNumber(test1));
        
        // Test case 2: All zeros
        int[] test2 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        System.out.println("Test 2: " + createPhoneNumber(test2));
        
        // Test case 3: All nines
        int[] test3 = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
        System.out.println("Test 3: " + createPhoneNumber(test3));
        
        // Test case 4: Mixed numbers
        int[] test4 = {5, 5, 5, 1, 2, 3, 4, 5, 6, 7};
        System.out.println("Test 4: " + createPhoneNumber(test4));
        
        // Test case 5: Another mixed pattern
        int[] test5 = {8, 0, 0, 5, 5, 5, 1, 2, 3, 4};
        System.out.println("Test 5: " + createPhoneNumber(test5));
    }
}
