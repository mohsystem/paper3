public class Lse4 {
    /**
     * Securely retrieves a vehicle from an array by index.
     *
     * @param vehicles The array of vehicle names.
     * @param index    The index of the vehicle to retrieve.
     * @return The vehicle name at the given index or an error message if the index is invalid.
     */
    public static String getVehicle(String[] vehicles, int index) {
        // Security check: Ensure the vehicles array is not null.
        if (vehicles == null) {
            return "Error: Vehicle array is null.";
        }
        
        // Security check: Validate that the index is within the legitimate bounds of the array.
        // This prevents ArrayIndexOutOfBoundsException.
        if (index >= 0 && index < vehicles.length) {
            return vehicles[index];
        } else {
            return "Error: Index is out of bounds.";
        }
    }

    public static void main(String[] args) {
        String[] vehicleArray = {"Car", "Motorcycle", "Truck", "Bus", "Bicycle"};

        System.out.println("--- 5 Test Cases ---");

        // Test cases with various indices
        int[] testIndices = {0, 2, 4, -1, 5};

        // Test Case 1: Valid index (lower bound)
        System.out.println("Requesting vehicle at index " + testIndices[0] + ": " + getVehicle(vehicleArray, testIndices[0]));

        // Test Case 2: Valid index (middle)
        System.out.println("Requesting vehicle at index " + testIndices[1] + ": " + getVehicle(vehicleArray, testIndices[1]));

        // Test Case 3: Valid index (upper bound)
        System.out.println("Requesting vehicle at index " + testIndices[2] + ": " + getVehicle(vehicleArray, testIndices[2]));

        // Test Case 4: Invalid index (negative)
        System.out.println("Requesting vehicle at index " + testIndices[3] + ": " + getVehicle(vehicleArray, testIndices[3]));

        // Test Case 5: Invalid index (too high)
        System.out.println("Requesting vehicle at index " + testIndices[4] + ": " + getVehicle(vehicleArray, testIndices[4]));
    }
}