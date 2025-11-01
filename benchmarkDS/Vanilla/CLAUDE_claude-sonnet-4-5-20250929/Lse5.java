
public class Lse5 {
    public static String getElementAtIndex(int index) {
        String[] elements = {"apple", "banana", "cherry", "date", "elderberry"};
        
        if (index >= 0 && index < elements.length) {
            return elements[index];
        } else {
            return "Index out of bounds";
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test case 1: " + getElementAtIndex(0));
        System.out.println("Test case 2: " + getElementAtIndex(2));
        System.out.println("Test case 3: " + getElementAtIndex(4));
        System.out.println("Test case 4: " + getElementAtIndex(-1));
        System.out.println("Test case 5: " + getElementAtIndex(10));
        
        // Command line argument handling
        if (args.length > 0) {
            try {
                int index = Integer.parseInt(args[0]);
                System.out.println("Element at index " + index + ": " + getElementAtIndex(index));
            } catch (NumberFormatException e) {
                System.out.println("Invalid input. Please provide an integer.");
            }
        }
    }
}
