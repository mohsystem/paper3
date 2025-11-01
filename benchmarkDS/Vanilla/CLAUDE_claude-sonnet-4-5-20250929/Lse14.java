
public class Lse14 {
    private static int[] id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    public static int getValueAtIndex(int index) {
        if (index >= 0 && index < id_sequence.length) {
            return id_sequence[index];
        }
        throw new IndexOutOfBoundsException("Index out of bounds");
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid index at beginning
        try {
            System.out.println("Test 1 (index 0): " + getValueAtIndex(0));
        } catch (Exception e) {
            System.out.println("Test 1 failed: " + e.getMessage());
        }
        
        // Test case 2: Valid index in middle
        try {
            System.out.println("Test 2 (index 5): " + getValueAtIndex(5));
        } catch (Exception e) {
            System.out.println("Test 2 failed: " + e.getMessage());
        }
        
        // Test case 3: Valid index at end
        try {
            System.out.println("Test 3 (index 9): " + getValueAtIndex(9));
        } catch (Exception e) {
            System.out.println("Test 3 failed: " + e.getMessage());
        }
        
        // Test case 4: Invalid negative index
        try {
            System.out.println("Test 4 (index -1): " + getValueAtIndex(-1));
        } catch (Exception e) {
            System.out.println("Test 4 failed: " + e.getMessage());
        }
        
        // Test case 5: Invalid index beyond array length
        try {
            System.out.println("Test 5 (index 15): " + getValueAtIndex(15));
        } catch (Exception e) {
            System.out.println("Test 5 failed: " + e.getMessage());
        }
    }
}
