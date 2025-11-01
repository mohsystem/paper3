public class Lse4 {

    /**
     * Accesses an array of vehicles at a given index.
     *
     * @param vehicles An array of vehicle names.
     * @param index    The index to access.
     * @return The vehicle name at the specified index, or an error message if the index is out of bounds.
     */
    public static String getVehicleByIndex(String[] vehicles, int index) {
        if (index >= 0 && index < vehicles.length) {
            return vehicles[index];
        } else {
            return "Error: Index is out of bounds.";
        }
    }

    public static void main(String[] args) {
        String[] vehicles = {"car", "bus", "bike", "train", "plane"};

        // Test cases
        System.out.println("Index 0: " + getVehicleByIndex(vehicles, 0));  // Expected: car
        System.out.println("Index 2: " + getVehicleByIndex(vehicles, 2));  // Expected: bike
        System.out.println("Index 4: " + getVehicleByIndex(vehicles, 4));  // Expected: plane
        System.out.println("Index -1: " + getVehicleByIndex(vehicles, -1)); // Expected: Error
        System.out.println("Index 5: " + getVehicleByIndex(vehicles, 5));  // Expected: Error
    }
}