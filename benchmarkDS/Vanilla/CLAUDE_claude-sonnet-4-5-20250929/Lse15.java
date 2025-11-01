
public class Lse15 {
    private static final int[] id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    public static int getValueAtIndex(int index) {
        if (index >= 0 && index < id_sequence.length) {
            return id_sequence[index];
        }
        throw new IndexOutOfBoundsException("Index out of bounds");
    }
    
    public static void main(String[] args) {
        // Test case 1
        try {
            System.out.println("Test 1 - Index 0: " + getValueAtIndex(0));
        } catch (Exception e) {
            System.out.println("Test 1 - Error: " + e.getMessage());
        }
        
        // Test case 2
        try {
            System.out.println("Test 2 - Index 5: " + getValueAtIndex(5));
        } catch (Exception e) {
            System.out.println("Test 2 - Error: " + e.getMessage());
        }
        
        // Test case 3
        try {
            System.out.println("Test 3 - Index 9: " + getValueAtIndex(9));
        } catch (Exception e) {
            System.out.println("Test 3 - Error: " + e.getMessage());
        }
        
        // Test case 4
        try {
            System.out.println("Test 4 - Index -1: " + getValueAtIndex(-1));
        } catch (Exception e) {
            System.out.println("Test 4 - Error: " + e.getMessage());
        }
        
        // Test case 5
        try {
            System.out.println("Test 5 - Index 15: " + getValueAtIndex(15));
        } catch (Exception e) {
            System.out.println("Test 5 - Error: " + e.getMessage());
        }
    }
}
