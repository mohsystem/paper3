
public class Lse13 {
    private static final int[] id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    public static Integer getValueAtIndex(int index) {
        if (index < 0 || index >= id_sequence.length) {
            return null; // Return null for invalid index
        }
        return id_sequence[index];
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid index at start
        System.out.println("Test 1 (index 0): " + getValueAtIndex(0));
        
        // Test case 2: Valid index in middle
        System.out.println("Test 2 (index 5): " + getValueAtIndex(5));
        
        // Test case 3: Valid index at end
        System.out.println("Test 3 (index 9): " + getValueAtIndex(9));
        
        // Test case 4: Invalid negative index
        System.out.println("Test 4 (index -1): " + getValueAtIndex(-1));
        
        // Test case 5: Invalid out of bounds index
        System.out.println("Test 5 (index 10): " + getValueAtIndex(10));
    }
}
