
public class Lse18 {
    private static final int[] array = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    public static Integer getValueAtIndex(int index) {
        if (index < 0 || index >= array.length) {
            return null; // Return null for invalid index (secure approach)
        }
        return array[index];
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid index in middle
        System.out.println("Test 1 - Index 5: " + getValueAtIndex(5));
        
        // Test case 2: Valid index at start
        System.out.println("Test 2 - Index 0: " + getValueAtIndex(0));
        
        // Test case 3: Valid index at end
        System.out.println("Test 3 - Index 9: " + getValueAtIndex(9));
        
        // Test case 4: Invalid index (negative)
        System.out.println("Test 4 - Index -1: " + getValueAtIndex(-1));
        
        // Test case 5: Invalid index (out of bounds)
        System.out.println("Test 5 - Index 15: " + getValueAtIndex(15));
    }
}
