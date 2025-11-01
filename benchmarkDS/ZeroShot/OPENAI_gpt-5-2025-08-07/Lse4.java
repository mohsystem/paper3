import java.util.Arrays;

public class Lse4 {
    private static final String[] VEHICLES = {"Car", "Truck", "Bike", "Bus", "Van"};

    public static String getVehicleByIndex(String indexInput) {
        if (indexInput == null) {
            return "Invalid input: null";
        }
        String s = indexInput.trim();
        if (s.isEmpty()) {
            return "Invalid input: empty";
        }
        // Validate integer format
        if (!s.matches("[+-]?\\d+")) {
            return "Invalid input: not an integer";
        }
        try {
            long idx = Long.parseLong(s);
            if (idx < 0 || idx > Integer.MAX_VALUE) {
                return "Invalid index: out of range";
            }
            int i = (int) idx;
            if (i >= 0 && i < VEHICLES.length) {
                return VEHICLES[i];
            } else {
                return "Invalid index: must be between 0 and " + (VEHICLES.length - 1);
            }
        } catch (NumberFormatException e) {
            return "Invalid input: not an integer";
        }
    }

    public static void main(String[] args) {
        String[] tests = {"2", " 0 ", "-1", "10", "abc"};
        for (String t : tests) {
            String result = getVehicleByIndex(t);
            System.out.println("Input: '" + t + "' => " + result);
        }
    }
}