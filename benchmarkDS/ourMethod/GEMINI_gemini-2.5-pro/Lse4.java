import java.util.InputMismatchException;
import java.util.Scanner;

public class Lse4 {

    private static final String[] VEHICLES = {"Car", "Bus", "Motorcycle", "Airplane", "Boat"};

    /**
     * Retrieves a vehicle from the array by its index.
     *
     * @param index The index of the vehicle to retrieve.
     * @return The vehicle name if the index is valid, otherwise an error message.
     */
    public static String getVehicle(int index) {
        // Rule #1, #4: Ensure array access is within boundaries.
        if (index >= 0 && index < VEHICLES.length) {
            return VEHICLES[index];
        } else {
            return "Error: Index is out of bounds.";
        }
    }

    /**
     * Processes a string input, converts it to an integer, and gets the vehicle.
     * This function encapsulates parsing and validation.
     *
     * @param input The string input representing the index.
     * @return The result of the getVehicle call or an error message if parsing fails.
     */
    public static String processInput(String input) {
        try {
            // Rule #4: Validate input by parsing to the expected type.
            int index = Integer.parseInt(input);
            return getVehicle(index);
        } catch (NumberFormatException e) {
            // Rule #7: Handle exceptions for invalid number formats.
            return "Error: Input is not a valid integer.";
        }
    }

    public static void main(String[] args) {
        // In a real application, you would use a Scanner to get user input:
        // Scanner scanner = new Scanner(System.in);
        // System.out.print("Enter an index of a vehicle: ");
        // String userInput = scanner.nextLine();
        // System.out.println("Vehicle: " + processInput(userInput));
        // scanner.close();

        // For demonstration, here are 5 hardcoded test cases.
        String[] testInputs = {"2", "0", "4", "10", "-1", "Car"};
        
        System.out.println("Running test cases...");
        for (int i = 0; i < testInputs.length; i++) {
            System.out.println("\nTest Case " + (i + 1) + ":");
            System.out.println("Input: \"" + testInputs[i] + "\"");
            String result = processInput(testInputs[i]);
            System.out.println("Output: " + result);
        }
    }
}