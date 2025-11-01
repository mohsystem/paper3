
public class Lse18 {
    private static int[] array = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    public static int getValueAtIndex(int index) {
        if (index >= 0 && index < array.length) {
            return array[index];
        }
        throw new IndexOutOfBoundsException("Index out of bounds");
    }
    
    public static void main(String[] args) {
        // Test case 1
        try {
            System.out.println("Index 5: " + getValueAtIndex(5));
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        // Test case 2
        try {
            System.out.println("Index 0: " + getValueAtIndex(0));
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        // Test case 3
        try {
            System.out.println("Index 9: " + getValueAtIndex(9));
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        // Test case 4
        try {
            System.out.println("Index 3: " + getValueAtIndex(3));
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        // Test case 5
        try {
            System.out.println("Index 10: " + getValueAtIndex(10));
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
    }
}
