
public class Lse13 {
    private static int[] id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    public static int getValueAtIndex(int index) {
        if (index >= 0 && index < id_sequence.length) {
            return id_sequence[index];
        }
        return -1; // Return -1 for invalid index
    }
    
    public static void main(String[] args) {
        // Test case 1
        int index1 = 0;
        System.out.println("Index " + index1 + ": " + getValueAtIndex(index1));
        
        // Test case 2
        int index2 = 5;
        System.out.println("Index " + index2 + ": " + getValueAtIndex(index2));
        
        // Test case 3
        int index3 = 9;
        System.out.println("Index " + index3 + ": " + getValueAtIndex(index3));
        
        // Test case 4
        int index4 = -1;
        System.out.println("Index " + index4 + ": " + getValueAtIndex(index4));
        
        // Test case 5
        int index5 = 10;
        System.out.println("Index " + index5 + ": " + getValueAtIndex(index5));
    }
}
