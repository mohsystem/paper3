import java.nio.charset.StandardCharsets;

public class Lse4 {
    private static final String[] VEHICLES = {
        "Car", "Bike", "Truck", "Bus", "Scooter",
        "Van", "Train", "Boat", "Plane", "Tractor"
    };

    // Returns the vehicle at the given index as a String, or an error message.
    public static String getVehicleByIndex(String input) {
        if (input == null) {
            return "Invalid input";
        }

        // Normalize and validate length
        String trimmed = input.trim();
        if (trimmed.isEmpty() || trimmed.length() > 20) {
            return "Invalid input";
        }

        // Validate numeric format: optional +/-, then digits
        if (!trimmed.matches("^[+-]?\\d{1,10}$")) {
            return "Invalid input";
        }

        try {
            long value = Long.parseLong(trimmed);
            if (value < Integer.MIN_VALUE || value > Integer.MAX_VALUE) {
                return "Invalid input";
            }
            int idx = (int) value;
            if (idx < 0 || idx >= VEHICLES.length) {
                return "Index out of range";
            }
            return VEHICLES[idx];
        } catch (NumberFormatException e) {
            return "Invalid input";
        }
    }

    public static void main(String[] args) {
        String[] tests = {"0", "3", "10", "-1", "2"};
        for (String t : tests) {
            System.out.println("Input: " + t + " -> Output: " + getVehicleByIndex(t));
        }
    }
}