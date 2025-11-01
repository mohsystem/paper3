public class Lse6 {

    /**
     * Securely retrieves a vehicle from the array by index.
     * @param vehicles The array of vehicles.
     * @param index The index to retrieve.
     * @return The vehicle at the specified index or an error message if the index is invalid.
     */
    public static String getVehicleAtIndex(String[] vehicles, int index) {
        // Security check: Ensure the index is within the valid range.
        if (index >= 0 && index < vehicles.length) {
            return vehicles[index];
        } else {
            return "Error: Invalid index provided.";
        }
    }

    /**
     * Loops through the array and prints each vehicle.
     * @param vehicles The array of vehicles to print.
     */
    public static void printAllVehicles(String[] vehicles) {
        System.out.println("\n--- All Vehicles ---");
        for (String vehicle : vehicles) {
            System.out.println(vehicle);
        }
        System.out.println("--------------------");
    }

    public static void main(String[] args) {
        String[] vehicles = {"Car", "Bike", "Bus", "Train", "Plane"};

        // --- 5 Test Cases ---
        System.out.println("--- Test Cases ---");

        // Test Case 1: Valid index (first element)
        int index1 = 0;
        System.out.println("Requesting vehicle at index " + index1 + ": " + getVehicleAtIndex(vehicles, index1));

        // Test Case 2: Valid index (middle element)
        int index2 = 2;
        System.out.println("Requesting vehicle at index " + index2 + ": " + getVehicleAtIndex(vehicles, index2));

        // Test Case 3: Valid index (last element)
        int index3 = 4;
        System.out.println("Requesting vehicle at index " + index3 + ": " + getVehicleAtIndex(vehicles, index3));

        // Test Case 4: Invalid index (out of bounds - too high)
        int index4 = 10;
        System.out.println("Requesting vehicle at index " + index4 + ": " + getVehicleAtIndex(vehicles, index4));

        // Test Case 5: Invalid index (out of bounds - negative)
        int index5 = -1;
        System.out.println("Requesting vehicle at index " + index5 + ": " + getVehicleAtIndex(vehicles, index5));
        System.out.println("------------------");

        // Loop through and print all vehicles
        printAllVehicles(vehicles);
    }
}