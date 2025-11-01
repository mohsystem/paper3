
import java.util.Scanner;

public class Lse6 {
    public static String getVehicle(String[] vehicles, int index) {
        if (index >= 0 && index < vehicles.length) {
            return vehicles[index];
        }
        return "Invalid index";
    }
    
    public static void printAllVehicles(String[] vehicles) {
        for (String vehicle : vehicles) {
            System.out.println(vehicle);
        }
    }
    
    public static void main(String[] args) {
        String[] vehicles = {"Car", "Truck", "Motorcycle", "Bus", "Bicycle"};
        
        // Test cases
        System.out.println("Test Case 1: Index 0");
        System.out.println("Vehicle: " + getVehicle(vehicles, 0));
        System.out.println();
        
        System.out.println("Test Case 2: Index 2");
        System.out.println("Vehicle: " + getVehicle(vehicles, 2));
        System.out.println();
        
        System.out.println("Test Case 3: Index 4");
        System.out.println("Vehicle: " + getVehicle(vehicles, 4));
        System.out.println();
        
        System.out.println("Test Case 4: Invalid index -1");
        System.out.println("Vehicle: " + getVehicle(vehicles, -1));
        System.out.println();
        
        System.out.println("Test Case 5: Invalid index 10");
        System.out.println("Vehicle: " + getVehicle(vehicles, 10));
        System.out.println();
        
        System.out.println("All vehicles:");
        printAllVehicles(vehicles);
    }
}
