
public class Lse18 {
    private static final int[] array = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    public static Integer getValueAtIndex(int index) {
        // Input validation: check if index is negative
        if (index < 0) {
            System.out.println("Error: Index cannot be negative");
            return null;
        }
        
        // Boundary check: ensure index is within array bounds
        if (index >= array.length) {
            System.out.println("Error: Index out of bounds");
            return null;
        }
        
        return array[index];
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid index
        System.out.println("Test 1 - Index 5: " + getValueAtIndex(5));
        
        // Test case 2: First element
        System.out.println("Test 2 - Index 0: " + getValueAtIndex(0));
        
        // Test case 3: Last element
        System.out.println("Test 3 - Index 9: " + getValueAtIndex(9));
        
        // Test case 4: Out of bounds
        System.out.println("Test 4 - Index 15: " + getValueAtIndex(15));
        
        // Test case 5: Negative index
        System.out.println("Test 5 - Index -1: " + getValueAtIndex(-1));
    }
}
