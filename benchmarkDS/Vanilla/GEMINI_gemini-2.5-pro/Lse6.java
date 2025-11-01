public class Lse6 {

    /**
     * Returns the vehicle at a given index from the array.
     * @param vehicles The array of vehicles.
     * @param index The index of the vehicle to retrieve.
     * @return The vehicle string or "Invalid index" if the index is out of bounds.
     */
    public static String getVehicle(String[] vehicles, int index) {
        if (index >= 0 && index < vehicles.length) {
            return vehicles[index];
        } else {
            return "Invalid index";
        }
    }

    /**
     * Loops through the array and prints each vehicle.
     * @param vehicles The array of vehicles to print.
     */
    public static void printAllVehicles(String[] vehicles) {
        System.out.println("All vehicles:");
        for (String vehicle : vehicles) {
            System.out.println(vehicle);
        }
        System.out.println();
    }

    public static void main(String[] args) {
        // Create an array of strings called vehicles
        String[] vehicles = {"Car", "Bike", "Bus", "Train", "Plane"};

        // Loop through the array and print out each vehicle
        printAllVehicles(vehicles);

        // 5 test cases for getting a vehicle by index
        System.out.println("--- Test Cases for getVehicle ---");

        // Test Case 1
        int index1 = 0;
        System.out.println("Vehicle at index " + index1 + ": " + getVehicle(vehicles, index1));

        // Test Case 2
        int index2 = 2;
        System.out.println("Vehicle at index " + index2 + ": " + getVehicle(vehicles, index2));

        // Test Case 3
        int index3 = 4;
        System.out.println("Vehicle at index " + index3 + ": " + getVehicle(vehicles, index3));

        // Test Case 4 (out of bounds - upper)
        int index4 = 5;
        System.out.println("Vehicle at index " + index4 + ": " + getVehicle(vehicles, index4));

        // Test Case 5 (out of bounds - lower)
        int index5 = -1;
        System.out.println("Vehicle at index " + index5 + ": " + getVehicle(vehicles, index5));
    }
}