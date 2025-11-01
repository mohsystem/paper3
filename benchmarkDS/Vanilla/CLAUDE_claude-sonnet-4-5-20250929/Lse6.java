
import java.util.Scanner;

public class Lse6 {
    public static String getVehicle(String[] vehicles, int index) {
        if (index >= 0 && index < vehicles.length) {
            return vehicles[index];
        }
        return "Invalid index";
    }
    
    public static void printAllVehicles(String[] vehicles) {
        for (int i = 0; i < vehicles.length; i++) {
            System.out.println(vehicles[i]);
        }
    }
    
    public static void main(String[] args) {
        String[] vehicles = {"Car", "Truck", "Motorcycle", "Bus", "Bicycle"};
        
        // Test case 1: Valid index 0
        System.out.println("Test 1 - Index 0: " + getVehicle(vehicles, 0));
        
        // Test case 2: Valid index 2
        System.out.println("Test 2 - Index 2: " + getVehicle(vehicles, 2));
        
        // Test case 3: Valid index 4
        System.out.println("Test 3 - Index 4: " + getVehicle(vehicles, 4));
        
        // Test case 4: Invalid index -1
        System.out.println("Test 4 - Index -1: " + getVehicle(vehicles, -1));
        
        // Test case 5: Invalid index 10
        System.out.println("Test 5 - Index 10: " + getVehicle(vehicles, 10));
        
        System.out.println("\\nAll vehicles:");
        printAllVehicles(vehicles);
    }
}
