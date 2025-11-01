public class Lse6 {
    public static String getVehicleByIndex(String[] vehicles, int index) {
        if (vehicles == null || index < 0 || index >= vehicles.length) return "Invalid index";
        return vehicles[index];
    }

    public static void printVehicles(String[] vehicles) {
        if (vehicles == null) return;
        for (String v : vehicles) {
            System.out.println(v);
        }
    }

    public static void main(String[] args) {
        String[] vehicles = new String[] {"Car", "Bus", "Bike", "Truck", "Scooter"};

        int[] testIndices = new int[] {0, 2, 4, -1, 10};
        for (int idx : testIndices) {
            String result = getVehicleByIndex(vehicles, idx);
            System.out.println("Index " + idx + ": " + result);
        }

        System.out.println("All vehicles:");
        printVehicles(vehicles);
    }
}