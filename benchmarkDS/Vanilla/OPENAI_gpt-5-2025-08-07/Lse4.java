import java.util.*;

public class Lse4 {
    private static final String[] VEHICLES = {"Car", "Truck", "Bike", "Bus", "Van"};

    public static String getVehicle(String input) {
        try {
            int index = Integer.parseInt(input.trim());
            if (index >= 0 && index < VEHICLES.length) {
                return VEHICLES[index];
            } else {
                return "Invalid index";
            }
        } catch (NumberFormatException e) {
            return "Invalid integer";
        }
    }

    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        System.out.print("Enter index of vehicle: ");
        String userInput = sc.nextLine();
        String result = getVehicle(userInput);
        System.out.println(result);

        // 5 test cases
        String[] tests = {"0", "1", "2", "3", "4"};
        for (String t : tests) {
            System.out.println("Input: " + t + " -> " + getVehicle(t));
        }
        sc.close();
    }
}