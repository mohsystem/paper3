
public class Lse16 {
    private static final int[] array = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    public static int getValueAtIndex(int index) {
        if (index < 0 || index >= array.length) {
            throw new IndexOutOfBoundsException("Index out of bounds: " + index);
        }
        return array[index];
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println("Index 0: " + getValueAtIndex(0));
        
        // Test case 2
        System.out.println("Index 5: " + getValueAtIndex(5));
        
        // Test case 3
        System.out.println("Index 9: " + getValueAtIndex(9));
        
        // Test case 4
        System.out.println("Index 3: " + getValueAtIndex(3));
        
        // Test case 5
        System.out.println("Index 7: " + getValueAtIndex(7));
    }
}
