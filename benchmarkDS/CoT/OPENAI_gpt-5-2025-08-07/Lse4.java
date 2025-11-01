public class Lse4 {
    private static final String[] VEHICLES = {"Car", "Bus", "Truck", "Bike", "Scooter", "Van", "SUV"};

    // Accepts the index input as a parameter, returns the vehicle or an error message
    public static String getVehicleAtIndex(String indexInput) {
        if (indexInput == null) {
            return "Error: index input is null";
        }
        String s = indexInput.trim();
        if (s.isEmpty()) {
            return "Error: empty index input";
        }
        int index;
        try {
            index = Integer.parseInt(s);
        } catch (NumberFormatException e) {
            return "Error: invalid index format";
        }
        if (index < 0 || index >= VEHICLES.length) {
            return "Error: index out of bounds";
        }
        return VEHICLES[index];
    }

    // Main method with 5 test cases
    public static void main(String[] args) {
        String[] tests = {"0", "2", "6", "-1", "abc"};
        for (String t : tests) {
            String result = getVehicleAtIndex(t);
            System.out.println("Input: \"" + t + "\" -> " + result);
        }
    }
}