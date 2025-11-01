import java.util.InputMismatchException;

public class Lse6 {

    /**
     * Retrieves a vehicle from the array at the specified index.
     *
     * @param vehicles The array of vehicles.
     * @param index The index of the vehicle to retrieve.
     * @return The vehicle string at the given index, or an error message if the index is invalid.
     */
    public static String getVehicle(String[] vehicles, int index) {
        // Rule #1, #3, #4: Input validation - check if the index is within the valid bounds.
        if (vehicles != null && index >= 0 && index < vehicles.length) {
            return vehicles[index];
        } else {
            return "Error: Index out of bounds.";
        }
    }

    /**
     * Prints all vehicles in the given array to the console.
     *
     * @param vehicles The array of vehicles to print.
     */
    public static void printAllVehicles(String[] vehicles) {
        if (vehicles == null) {
            System.out.println("Vehicle array is null.");
            return;
        }
        System.out.println("All vehicles:");
        for (String vehicle : vehicles) {
            System.out.println(vehicle);
        }
    }

    public static void main(String[] args) {
        // The array of strings called vehicles.
        String[] vehicles = {"Car", "Bus", "Motorcycle", "Bicycle", "Truck"};

        // Loop through the array and print out each vehicle.
        printAllVehicles(vehicles);
        System.out.println("--------------------");

        // 5 test cases
        System.out.println("Running test cases...");

        // Test Case 1: Valid index (0)
        int testIndex1 = 0;
        System.out.println("Getting vehicle at index " + testIndex1 + ": " + getVehicle(vehicles, testIndex1));

        // Test Case 2: Valid index (middle)
        int testIndex2 = 2;
        System.out.println("Getting vehicle at index " + testIndex2 + ": " + getVehicle(vehicles, testIndex2));
        
        // Test Case 3: Valid index (last element)
        int testIndex3 = 4;
        System.out.println("Getting vehicle at index " + testIndex3 + ": " + getVehicle(vehicles, testIndex3));

        // Test Case 4: Invalid index (negative)
        int testIndex4 = -1;
        System.out.println("Getting vehicle at index " + testIndex4 + ": " + getVehicle(vehicles, testIndex4));

        // Test Case 5: Invalid index (out of bounds)
        int testIndex5 = 5;
        System.out.println("Getting vehicle at index " + testIndex5 + ": " + getVehicle(vehicles, testIndex5));
    }
}