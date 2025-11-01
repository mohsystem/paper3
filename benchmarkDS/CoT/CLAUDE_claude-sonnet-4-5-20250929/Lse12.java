
public class Lse12 {
    public static int getValueFromArray(int[] array, int size, int index) {
        // Security check: validate index to prevent out-of-bounds access
        if (array == null || index < 0 || index >= size || index >= array.length) {
            return -1; // Return error code for invalid access
        }
        return array[index];
    }
    
    public static void main(String[] args) {
        // Test cases
        int[] testArray1 = {10, 20, 30, 40, 50};
        
        // Test case 1: Valid index in middle
        System.out.println("Test 1: " + getValueFromArray(testArray1, 5, 2)); // Expected: 30
        
        // Test case 2: Valid index at start
        System.out.println("Test 2: " + getValueFromArray(testArray1, 5, 0)); // Expected: 10
        
        // Test case 3: Valid index at end
        System.out.println("Test 3: " + getValueFromArray(testArray1, 5, 4)); // Expected: 50
        
        // Test case 4: Invalid index (negative)
        System.out.println("Test 4: " + getValueFromArray(testArray1, 5, -1)); // Expected: -1
        
        // Test case 5: Invalid index (out of bounds)
        System.out.println("Test 5: " + getValueFromArray(testArray1, 5, 10)); // Expected: -1
        
        // Original functionality: read from standard input
        try {
            java.util.Scanner scanner = new java.util.Scanner(System.in);
            if (scanner.hasNextInt()) {
                int index = scanner.nextInt();
                int result = getValueFromArray(testArray1, testArray1.length, index);
                System.exit(result >= 0 ? result : 1);
            }
            scanner.close();
        } catch (Exception e) {
            System.exit(1);
        }
    }
}
