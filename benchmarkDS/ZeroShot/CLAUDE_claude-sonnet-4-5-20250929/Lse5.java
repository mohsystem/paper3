
public class Lse5 {
    public static String getElementAtIndex(int index) {
        String[] elements = {"apple", "banana", "cherry", "date", "elderberry"};
        
        if (index < 0 || index >= elements.length) {
            return "Error: Index out of bounds";
        }
        
        return elements[index];
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1: " + getElementAtIndex(0));
        System.out.println("Test Case 2: " + getElementAtIndex(2));
        System.out.println("Test Case 3: " + getElementAtIndex(4));
        System.out.println("Test Case 4: " + getElementAtIndex(-1));
        System.out.println("Test Case 5: " + getElementAtIndex(10));
        
        // Command line argument processing
        if (args.length > 0) {
            try {
                int index = Integer.parseInt(args[0]);
                System.out.println("\\nCommand line result: " + getElementAtIndex(index));
            } catch (NumberFormatException e) {
                System.out.println("Error: Invalid number format");
            }
        }
    }
}
