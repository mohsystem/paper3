
public class Lse10 {
    public static int getValueFromArray(int[] array, int size, int index) {
        if (array == null || index < 0 || index >= size || index >= array.length) {
            return -1; // Return error code for invalid access
        }
        return array[index];
    }
    
    public static void main(String[] args) {
        int[] id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        int size = id_sequence.length;
        
        // Test case 1: Valid index at beginning
        int index1 = 0;
        int result1 = getValueFromArray(id_sequence, size, index1);
        System.out.println("Test 1 - Index " + index1 + ": " + result1);
        
        // Test case 2: Valid index in middle
        int index2 = 5;
        int result2 = getValueFromArray(id_sequence, size, index2);
        System.out.println("Test 2 - Index " + index2 + ": " + result2);
        
        // Test case 3: Valid index at end
        int index3 = 9;
        int result3 = getValueFromArray(id_sequence, size, index3);
        System.out.println("Test 3 - Index " + index3 + ": " + result3);
        
        // Test case 4: Invalid index (negative)
        int index4 = -1;
        int result4 = getValueFromArray(id_sequence, size, index4);
        System.out.println("Test 4 - Index " + index4 + ": " + result4);
        
        // Test case 5: Invalid index (out of bounds)
        int index5 = 15;
        int result5 = getValueFromArray(id_sequence, size, index5);
        System.out.println("Test 5 - Index " + index5 + ": " + result5);
        
        System.exit(result1);
    }
}
