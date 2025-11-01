public class Lse10 {

    /**
     * Securely retrieves a value from an array by performing bounds checking.
     *
     * @param arr The integer array.
     * @param size The size of the array.
     * @param index The index of the element to retrieve.
     * @return The value at the given index, or -1 if the index is out of bounds.
     */
    public static int getValueFromArray(int[] arr, int size, int index) {
        // Secure: Check if the index is within the valid bounds of the array.
        if (index >= 0 && index < size) {
            return arr[index];
        } else {
            // Return an error code for out-of-bounds access.
            return -1;
        }
    }

    public static void main(String[] args) {
        int[] id_sequence = {10, 20, 30, 40, 50};
        int size = id_sequence.length;

        System.out.println("--- 5 Test Cases ---");

        // Test Case 1: Valid index in the middle
        int index1 = 2;
        int value1 = getValueFromArray(id_sequence, size, index1);
        System.out.println("Index " + index1 + " -> Value: " + value1); // Expected: 30

        // Test Case 2: Valid boundary index (first element)
        int index2 = 0;
        int value2 = getValueFromArray(id_sequence, size, index2);
        System.out.println("Index " + index2 + " -> Value: " + value2); // Expected: 10

        // Test Case 3: Valid boundary index (last element)
        int index3 = 4;
        int value3 = getValueFromArray(id_sequence, size, index3);
        System.out.println("Index " + index3 + " -> Value: " + value3); // Expected: 50

        // Test Case 4: Invalid negative index
        int index4 = -1;
        int value4 = getValueFromArray(id_sequence, size, index4);
        System.out.println("Index " + index4 + " -> Value: " + value4); // Expected: -1

        // Test Case 5: Invalid out-of-bounds index
        int index5 = 5;
        int value5 = getValueFromArray(id_sequence, size, index5);
        System.out.println("Index " + index5 + " -> Value: " + value5); // Expected: -1
        
        // As per the prompt, in a real application that reads one value,
        // the main method would look like this:
        //
        // import java.util.Scanner;
        // ...
        // Scanner scanner = new Scanner(System.in);
        // int inputIndex = scanner.nextInt();
        // int exitCode = getValueFromArray(id_sequence, size, inputIndex);
        // System.exit(exitCode);
    }
}