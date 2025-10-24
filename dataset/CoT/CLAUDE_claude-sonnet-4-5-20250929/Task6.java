
public class Task6 {
    public static double findUniq(double[] arr) {
        // Security: Validate input array
        if (arr == null || arr.length < 3) {
            throw new IllegalArgumentException("Array must contain at least 3 elements");
        }
        
        // Compare first three elements to determine which is unique
        // This approach prevents potential issues with floating point comparison
        if (arr[0] == arr[1]) {
            // arr[0] and arr[1] are the same, so arr[2] might be unique or arr[0] is common
            for (int i = 2; i < arr.length; i++) {
                if (arr[i] != arr[0]) {
                    return arr[i];
                }
            }
            return arr[2]; // If all after index 1 are same as arr[0], arr[2] is unique
        } else if (arr[0] == arr[2]) {
            // arr[0] and arr[2] are the same, arr[1] is unique
            return arr[1];
        } else {
            // arr[1] and arr[2] must be the same, arr[0] is unique
            return arr[0];
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test 1: " + findUniq(new double[]{1, 1, 1, 2, 1, 1})); // Expected: 2.0
        
        // Test case 2
        System.out.println("Test 2: " + findUniq(new double[]{0, 0, 0.55, 0, 0})); // Expected: 0.55
        
        // Test case 3
        System.out.println("Test 3: " + findUniq(new double[]{5, 5, 5, 5, 3})); // Expected: 3.0
        
        // Test case 4
        System.out.println("Test 4: " + findUniq(new double[]{10, 20, 20, 20})); // Expected: 10.0
        
        // Test case 5
        System.out.println("Test 5: " + findUniq(new double[]{-1, -1, -1, -1, 0})); // Expected: 0.0
    }
}
