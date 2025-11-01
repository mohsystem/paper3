import java.util.Scanner;

class Lse6 {
    // Step 1: Problem understanding - Create vehicles array, get index from user, return and print vehicle, loop and print all.
    // Step 2: Security requirements - Validate inputs, handle invalid formats and out-of-bounds indexes.
    // Step 3 & 5: Secure coding and final secure output.

    public static String getVehicleAt(String[] vehicles, int index) {
        if (vehicles == null) return null;
        if (index < 0 || index >= vehicles.length) return null;
        return vehicles[index];
    }

    public static void printVehicles(String[] vehicles) {
        if (vehicles == null) return;
        for (int i = 0; i < vehicles.length; i++) {
            System.out.println("vehicles[" + i + "] = " + vehicles[i]);
        }
    }

    public static void main(String[] args) {
        String[] vehicles = new String[] {"Car", "Bike", "Truck", "Bus", "Scooter"};
        Scanner sc = new Scanner(System.in);
        try {
            System.out.print("Enter vehicle index (0-" + (vehicles.length - 1) + "): ");
            String line = sc.nextLine();
            int idx;
            try {
                idx = Integer.parseInt(line.trim());
            } catch (NumberFormatException e) {
                System.out.println("Invalid input. Please enter an integer.");
                idx = -1;
            }
            String selected = getVehicleAt(vehicles, idx);
            if (selected != null) {
                System.out.println("Selected vehicle: " + selected);
            } else {
                System.out.println("No vehicle found at the provided index.");
            }

            System.out.println("All vehicles:");
            printVehicles(vehicles);

            // 5 test cases
            System.out.println("\nTest cases:");
            int[] testIndices = {0, 2, 4, -1, 10};
            for (int i = 0; i < testIndices.length; i++) {
                String result = getVehicleAt(vehicles, testIndices[i]);
                System.out.println("Test " + (i + 1) + " - index " + testIndices[i] + ": " + (result != null ? result : "Invalid index"));
            }
        } finally {
            sc.close();
        }
    }
}