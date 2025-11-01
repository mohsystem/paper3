
public class Lse5 {
    public static void printElementAtIndex(String[] array, String indexStr) {
        if (indexStr == null || indexStr.trim().isEmpty()) {
            System.out.println("Error: Index argument is empty or null");
            return;
        }
        
        try {
            int index = Integer.parseInt(indexStr.trim());
            
            if (index < 0 || index >= array.length) {
                System.out.println("Error: Index " + index + " is out of bounds. Valid range: 0-" + (array.length - 1));
                return;
            }
            
            System.out.println("Element at index " + index + ": " + array[index]);
        } catch (NumberFormatException e) {
            System.out.println("Error: Invalid integer format: " + indexStr);
        }
    }
    
    public static void main(String[] args) {
        String[] elements = {"apple", "banana", "cherry", "date", "elderberry", "fig", "grape"};
        
        // Test cases
        System.out.println("Test Case 1: Valid index 0");
        printElementAtIndex(elements, "0");
        
        System.out.println("\\nTest Case 2: Valid index 3");
        printElementAtIndex(elements, "3");
        
        System.out.println("\\nTest Case 3: Negative index");
        printElementAtIndex(elements, "-1");
        
        System.out.println("\\nTest Case 4: Out of bounds index");
        printElementAtIndex(elements, "10");
        
        System.out.println("\\nTest Case 5: Invalid format");
        printElementAtIndex(elements, "abc");
    }
}
