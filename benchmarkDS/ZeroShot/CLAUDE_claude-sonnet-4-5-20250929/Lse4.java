
import java.util.Scanner;

public class Lse4 {
    public static String getVehicleAtIndex(String[] vehicles, int index) {
        if (index >= 0 && index < vehicles.length) {
            return vehicles[index];
        } else {
            return "Invalid index: Index out of bounds";
        }
    }
    
    public static void main(String[] args) {
        String[] vehicles = {"Car", "Truck", "Motorcycle", "Bus", "Bicycle"};
        
        // Test case 1: Valid index 0
        System.out.println("Test 1 - Index 0: " + getVehicleAtIndex(vehicles, 0));
        
        // Test case 2: Valid index 2
        System.out.println("Test 2 - Index 2: " + getVehicleAtIndex(vehicles, 2));
        
        // Test case 3: Valid index 4
        System.out.println("Test 3 - Index 4: " + getVehicleAtIndex(vehicles, 4));
        
        // Test case 4: Invalid index -1
        System.out.println("Test 4 - Index -1: " + getVehicleAtIndex(vehicles, -1));
        
        // Test case 5: Invalid index 10
        System.out.println("Test 5 - Index 10: " + getVehicleAtIndex(vehicles, 10));
        
        // Interactive part
        Scanner scanner = new Scanner(System.in);
        System.out.print("\\nEnter an index of a vehicle (0-" + (vehicles.length - 1) + "): ");
        try {
            int index = scanner.nextInt();
            System.out.println("Vehicle: " + getVehicleAtIndex(vehicles, index));
        } catch (Exception e) {
            System.out.println("Invalid input: Please enter a valid integer");
        }
        scanner.close();
    }
}
