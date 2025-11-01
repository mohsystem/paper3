public class Lse6 {

    /**
     * Retrieves a vehicle from the array at a specified index.
     * Includes bounds checking to prevent crashes.
     *
     * @param vehicles The array of vehicle strings.
     * @param index    The index of the vehicle to retrieve.
     * @return The vehicle string at the given index, or an error message if the index is invalid.
     */
    public static String getVehicle(String[] vehicles, int index) {
        if (index >= 0 && index < vehicles.length) {
            return vehicles[index];
        } else {
            return "Error: Index " + index + " is out of bounds.";
        }
    }

    /**
     * Loops through the array and prints each vehicle to the console.
     *
     * @param vehicles The array of vehicle strings to print.
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

        // Print all vehicles
        printAllVehicles(vehicles);

        // 5 Test Cases
        System.out.println("\n--- Test Cases ---");
        int[] testIndices = {1, 3, 0, -1, 5};

        for (int i = 0; i < testIndices.length; i++) {
            int index = testIndices[i];
            System.out.println("Test Case " + (i + 1) + ": Getting vehicle at index " + index);
            String vehicle = getVehicle(vehicles, index);
            System.out.println("Result: " + vehicle);
        }
    }
}