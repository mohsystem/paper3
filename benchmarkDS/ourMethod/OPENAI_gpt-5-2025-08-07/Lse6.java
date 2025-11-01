import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;

public class Lse6 {

    public static String getVehicle(String[] vehicles, int index) {
        if (vehicles == null) {
            return null;
        }
        if (index >= 0 && index < vehicles.length) {
            return vehicles[index];
        }
        return null;
    }

    public static void main(String[] args) {
        String[] vehicles = new String[] { "Car", "Bike", "Truck", "Bus", "Scooter" };

        // Prompt the user for an index and return that vehicle
        System.out.println("Available vehicles count: " + vehicles.length);
        System.out.print("Enter an index (0 to " + (vehicles.length - 1) + "): ");
        try (BufferedReader br = new BufferedReader(new InputStreamReader(System.in, StandardCharsets.UTF_8))) {
            String line = br.readLine();
            String trimmed = (line == null) ? "" : line.trim();
            if (trimmed.length() > 20) {
                System.out.println("Invalid index.");
            } else {
                try {
                    int idx = Integer.parseInt(trimmed);
                    String chosen = getVehicle(vehicles, idx);
                    if (chosen != null) {
                        System.out.println("Selected vehicle: " + chosen);
                    } else {
                        System.out.println("Invalid index.");
                    }
                } catch (NumberFormatException ex) {
                    System.out.println("Invalid index.");
                }
            }
        } catch (Exception e) {
            System.out.println("Invalid index.");
        }

        // Loop through the array and print out each vehicle
        System.out.println("All vehicles:");
        for (int i = 0; i < vehicles.length; i++) {
            System.out.println(i + ": " + vehicles[i]);
        }

        // 5 test cases
        System.out.println("Test cases:");
        int[] testIndices = new int[] { 0, 2, 4, -1, 10 };
        for (int idx : testIndices) {
            String result = getVehicle(vehicles, idx);
            if (result != null) {
                System.out.println("Index " + idx + " -> " + result);
            } else {
                System.out.println("Index " + idx + " -> Invalid index.");
            }
        }
    }
}