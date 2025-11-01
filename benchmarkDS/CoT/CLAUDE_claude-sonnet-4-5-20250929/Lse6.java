
import java.util.Scanner;

public class Lse6 {
    public static String getVehicleAtIndex(String[] vehicles, int index) {
        // Input validation and bounds checking
        if (vehicles == null || vehicles.length == 0) {
            return "Error: Vehicle array is empty or null";
        }
        
        if (index < 0 || index >= vehicles.length) {
            return "Error: Index out of bounds. Valid range: 0-" + (vehicles.length - 1);
        }
        
        return vehicles[index];
    }
    
    public static void printAllVehicles(String[] vehicles) {
        if (vehicles == null || vehicles.length == 0) {
            System.out.println("No vehicles to display.");
            return;
        }
        
        System.out.println("\\nAll vehicles:");
        for (int i = 0; i < vehicles.length; i++) {
            System.out.println("Index " + i + ": " + vehicles[i]);
        }
    }
    
    public static void main(String[] args) {
        String[] vehicles = {"Car", "Truck", "Motorcycle", "Bus", "Bicycle"};
        
        // Test cases
        System.out.println("=== Test Case 1: Valid index 0 ===");
        System.out.println("Vehicle at index 0: " + getVehicleAtIndex(vehicles, 0));
        printAllVehicles(vehicles);
        
        System.out.println("\\n=== Test Case 2: Valid index 3 ===");
        System.out.println("Vehicle at index 3: " + getVehicleAtIndex(vehicles, 3));
        printAllVehicles(vehicles);
        
        System.out.println("\\n=== Test Case 3: Invalid negative index ===");
        System.out.println("Vehicle at index -1: " + getVehicleAtIndex(vehicles, -1));
        printAllVehicles(vehicles);
        
        System.out.println("\\n=== Test Case 4: Invalid index too large ===");
        System.out.println("Vehicle at index 10: " + getVehicleAtIndex(vehicles, 10));
        printAllVehicles(vehicles);
        
        System.out.println("\\n=== Test Case 5: Last valid index ===");
        System.out.println("Vehicle at index 4: " + getVehicleAtIndex(vehicles, 4));
        printAllVehicles(vehicles);
    }
}
