public class Lse4 {

    private static final String[] VEHICLES = {"Car", "Bike", "Bus", "Truck", "Van"};

    /**
     * Gets a vehicle from the predefined list by its index.
     *
     * @param index The index of the vehicle to retrieve.
     * @return The name of the vehicle as a String, or "Invalid index" if the index is out of bounds.
     */
    public static String getVehicleByIndex(int index) {
        // Security Check: Validate that the index is within the valid range.
        if (index < 0 || index >= VEHICLES.length) {
            return "Invalid index";
        }
        return VEHICLES[index];
    }

    public static void main(String[] args) {
        // 5 Test Cases
        int[] testIndexes = {0, 2, 4, -1, 5};

        System.out.println("Running Java Test Cases:");
        for (int index : testIndexes) {
            String vehicle = getVehicleByIndex(index);
            System.out.println("Index: " + index + " -> Vehicle: " + vehicle);
        }
    }
}