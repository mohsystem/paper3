import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.Optional;

public class Lse6 {
    public static Optional<String> getVehicle(String[] vehicles, int index) {
        if (vehicles == null) return Optional.empty();
        if (index < 0 || index >= vehicles.length) return Optional.empty();
        return Optional.ofNullable(vehicles[index]);
    }

    public static void printVehicles(String[] vehicles) {
        if (vehicles == null) return;
        for (int i = 0; i < vehicles.length; i++) {
            System.out.println("Vehicle[" + i + "]: " + vehicles[i]);
        }
    }

    public static void main(String[] args) {
        String[] vehicles = new String[] { "Car", "Bike", "Truck", "Boat", "Plane" };

        // Optional interactive prompt controlled by environment variable
        String runInteractive = System.getenv("RUN_INTERACTIVE");
        if (runInteractive != null) {
            System.out.print("Enter vehicle index: ");
            try {
                BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
                String line = br.readLine();
                if (line != null) {
                    line = line.trim();
                    try {
                        int idx = Integer.parseInt(line);
                        Optional<String> result = getVehicle(vehicles, idx);
                        if (result.isPresent()) {
                            System.out.println("Selected vehicle: " + result.get());
                        } else {
                            System.out.println("Invalid index.");
                        }
                    } catch (NumberFormatException nfe) {
                        System.out.println("Invalid input. Please enter an integer index.");
                    }
                } else {
                    System.out.println("No input received.");
                }
            } catch (IOException e) {
                System.out.println("Error reading input.");
            }
        } else {
            System.out.println("RUN_INTERACTIVE not set; skipping interactive prompt.");
        }

        // 5 test cases
        int[] testIndices = new int[] { 0, 4, 2, -1, 10 };
        for (int idx : testIndices) {
            Optional<String> result = getVehicle(vehicles, idx);
            System.out.println("Test index " + idx + ": " + (result.isPresent() ? result.get() : "Invalid index"));
        }

        // Loop and print all vehicles
        printVehicles(vehicles);
    }
}