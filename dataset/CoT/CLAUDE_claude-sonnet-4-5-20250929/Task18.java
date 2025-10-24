
public class Task18 {
    public static int binaryArrayToInt(int[] binaryArray) {
        // Security: Validate input is not null
        if (binaryArray == null) {
            throw new IllegalArgumentException("Input array cannot be null");
        }
        
        // Security: Check for empty array
        if (binaryArray.length == 0) {
            return 0;
        }
        
        // Security: Validate array length to prevent integer overflow
        if (binaryArray.length > 31) {
            throw new IllegalArgumentException("Binary array too long, may cause integer overflow");
        }
        
        int result = 0;
        
        // Security: Validate each element and convert binary to integer
        for (int i = 0; i < binaryArray.length; i++) {
            // Security: Ensure only 0 or 1 values are present
            if (binaryArray[i] != 0 && binaryArray[i] != 1) {
                throw new IllegalArgumentException("Array must contain only 0 or 1 values");
            }
            
            // Shift left and add current bit
            result = (result << 1) | binaryArray[i];
        }
        
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] test1 = {0, 0, 0, 1};
        System.out.println("Test 1: [0, 0, 0, 1] => " + binaryArrayToInt(test1));
        
        // Test case 2
        int[] test2 = {0, 0, 1, 0};
        System.out.println("Test 2: [0, 0, 1, 0] => " + binaryArrayToInt(test2));
        
        // Test case 3
        int[] test3 = {0, 1, 0, 1};
        System.out.println("Test 3: [0, 1, 0, 1] => " + binaryArrayToInt(test3));
        
        // Test case 4
        int[] test4 = {1, 0, 0, 1};
        System.out.println("Test 4: [1, 0, 0, 1] => " + binaryArrayToInt(test4));
        
        // Test case 5
        int[] test5 = {1, 1, 1, 1};
        System.out.println("Test 5: [1, 1, 1, 1] => " + binaryArrayToInt(test5));
    }
}
