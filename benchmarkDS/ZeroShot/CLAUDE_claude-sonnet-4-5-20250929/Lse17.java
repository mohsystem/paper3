
import java.util.Scanner;

public class Lse17 {
    public static int getValueAtIndex(int index, int[] array) {
        if (index < 0 || index >= array.length) {
            throw new IndexOutOfBoundsException("Index out of bounds");
        }
        return array[index];
    }
    
    public static void main(String[] args) {
        int[] testArray = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        
        // Test case 1: Valid index at beginning
        try {
            System.out.println("Test 1 - Index 0: " + getValueAtIndex(0, testArray));
        } catch (Exception e) {
            System.out.println("Test 1 Error: " + e.getMessage());
        }
        
        // Test case 2: Valid index in middle
        try {
            System.out.println("Test 2 - Index 5: " + getValueAtIndex(5, testArray));
        } catch (Exception e) {
            System.out.println("Test 2 Error: " + e.getMessage());
        }
        
        // Test case 3: Valid index at end
        try {
            System.out.println("Test 3 - Index 9: " + getValueAtIndex(9, testArray));
        } catch (Exception e) {
            System.out.println("Test 3 Error: " + e.getMessage());
        }
        
        // Test case 4: Invalid negative index
        try {
            System.out.println("Test 4 - Index -1: " + getValueAtIndex(-1, testArray));
        } catch (Exception e) {
            System.out.println("Test 4 Error: " + e.getMessage());
        }
        
        // Test case 5: Invalid index beyond array length
        try {
            System.out.println("Test 5 - Index 15: " + getValueAtIndex(15, testArray));
        } catch (Exception e) {
            System.out.println("Test 5 Error: " + e.getMessage());
        }
    }
}
