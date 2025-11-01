
public class Lse16 {
    private static final int[] DATA_ARRAY = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    public static Integer getValueAtIndex(int index) {
        // Input validation: check for negative index
        if (index < 0) {
            System.out.println("Error: Index cannot be negative");
            return null;
        }
        
        // Bounds checking: ensure index is within array bounds
        if (index >= DATA_ARRAY.length) {
            System.out.println("Error: Index out of bounds");
            return null;
        }
        
        // Safe array access
        return DATA_ARRAY[index];
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid index at beginning
        System.out.println("Test 1 (index 0): " + getValueAtIndex(0));
        
        // Test case 2: Valid index in middle
        System.out.println("Test 2 (index 5): " + getValueAtIndex(5));
        
        // Test case 3: Valid index at end
        System.out.println("Test 3 (index 9): " + getValueAtIndex(9));
        
        // Test case 4: Invalid index (out of bounds)
        System.out.println("Test 4 (index 15): " + getValueAtIndex(15));
        
        // Test case 5: Invalid index (negative)
        System.out.println("Test 5 (index -1): " + getValueAtIndex(-1));
    }
}
