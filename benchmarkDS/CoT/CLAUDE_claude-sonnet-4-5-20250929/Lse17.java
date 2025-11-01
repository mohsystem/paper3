
public class Lse17 {
    private static final int[] dataArray = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    public static int getValueAtIndex(int index) {
        // Input validation - check if index is within valid bounds
        if (index < 0 || index >= dataArray.length) {
            System.out.println("Error: Index out of bounds. Valid range: 0-" + (dataArray.length - 1));
            return -1; // Return error value
        }
        return dataArray[index];
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid index at beginning
        System.out.println("Test 1 (index 0): " + getValueAtIndex(0));
        
        // Test case 2: Valid index in middle
        System.out.println("Test 2 (index 5): " + getValueAtIndex(5));
        
        // Test case 3: Valid index at end
        System.out.println("Test 3 (index 9): " + getValueAtIndex(9));
        
        // Test case 4: Invalid negative index
        System.out.println("Test 4 (index -1): " + getValueAtIndex(-1));
        
        // Test case 5: Invalid index beyond array
        System.out.println("Test 5 (index 15): " + getValueAtIndex(15));
    }
}
