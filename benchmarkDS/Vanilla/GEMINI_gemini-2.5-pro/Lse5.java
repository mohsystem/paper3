public class Lse5 {

    /**
     * Accepts an array and an index, then prints the element at that index.
     * It includes error handling for out-of-bounds indices.
     * @param data The integer array to access.
     * @param index The index of the element to print.
     */
    public static void getElementAtIndex(int[] data, int index) {
        try {
            System.out.println("Element at index " + index + ": " + data[index]);
        } catch (ArrayIndexOutOfBoundsException e) {
            System.out.println("Error: Index " + index + " is out of bounds for an array of size " + data.length + ".");
        }
    }

    public static void main(String[] args) {
        int[] elements = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

        // Check if a command-line argument is provided
        if (args.length > 0) {
            try {
                int indexFromArg = Integer.parseInt(args[0]);
                System.out.println("--- Running with command-line argument ---");
                getElementAtIndex(elements, indexFromArg);
            } catch (NumberFormatException e) {
                System.out.println("Error: Command-line argument must be a valid integer.");
            }
        } else {
            // If no command-line arguments, run the 5 built-in test cases
            System.out.println("--- Running built-in test cases ---");
            
            // Test Case 1: Accessing the first element
            getElementAtIndex(elements, 0);
            
            // Test Case 2: Accessing an element in the middle
            getElementAtIndex(elements, 4);

            // Test Case 3: Accessing the last element
            getElementAtIndex(elements, 9);
            
            // Test Case 4: Accessing with a negative index (out of bounds)
            getElementAtIndex(elements, -1);
            
            // Test Case 5: Accessing an index equal to the array length (out of bounds)
            getElementAtIndex(elements, 10);
        }
    }
}