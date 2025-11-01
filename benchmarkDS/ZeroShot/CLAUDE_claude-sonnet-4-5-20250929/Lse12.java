
public class Lse12 {
    public static int getValueFromArray(int[] array, int size, int index) {
        // Secure: Check if index is within valid bounds
        if (array == null || index < 0 || index >= size || index >= array.length) {
            return -1; // Return error code for invalid access
        }
        return array[index];
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid index in middle
        int[] test1 = {10, 20, 30, 40, 50};
        int result1 = getValueFromArray(test1, 5, 2);
        System.out.println("Test 1 - Expected: 30, Got: " + result1);
        
        // Test case 2: Valid index at start
        int[] test2 = {5, 15, 25, 35};
        int result2 = getValueFromArray(test2, 4, 0);
        System.out.println("Test 2 - Expected: 5, Got: " + result2);
        
        // Test case 3: Valid index at end
        int[] test3 = {100, 200, 300};
        int result3 = getValueFromArray(test3, 3, 2);
        System.out.println("Test 3 - Expected: 300, Got: " + result3);
        
        // Test case 4: Invalid index (out of bounds)
        int[] test4 = {1, 2, 3};
        int result4 = getValueFromArray(test4, 3, 5);
        System.out.println("Test 4 - Expected: -1 (error), Got: " + result4);
        
        // Test case 5: Invalid index (negative)
        int[] test5 = {7, 8, 9, 10};
        int result5 = getValueFromArray(test5, 4, -1);
        System.out.println("Test 5 - Expected: -1 (error), Got: " + result5);
        
        System.exit(result1);
    }
}
