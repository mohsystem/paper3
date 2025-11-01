
import java.util.Scanner;

public class Lse4 {
    public static String getVehicleAtIndex(String[] vehicles, int index) {
        if (index >= 0 && index < vehicles.length) {
            return vehicles[index];
        }
        return "Invalid index";
    }
    
    public static void main(String[] args) {
        String[] vehicles = {"Car", "Bike", "Truck", "Bus", "Van", "Motorcycle"};
        
        // Test case 1
        System.out.println("Test 1: " + getVehicleAtIndex(vehicles, 0));
        
        // Test case 2
        System.out.println("Test 2: " + getVehicleAtIndex(vehicles, 3));
        
        // Test case 3
        System.out.println("Test 3: " + getVehicleAtIndex(vehicles, 5));
        
        // Test case 4
        System.out.println("Test 4: " + getVehicleAtIndex(vehicles, -1));
        
        // Test case 5
        System.out.println("Test 5: " + getVehicleAtIndex(vehicles, 10));
        
        // Interactive mode
        Scanner scanner = new Scanner(System.in);
        System.out.print("\\nEnter an index of a vehicle: ");
        int index = scanner.nextInt();
        System.out.println(getVehicleAtIndex(vehicles, index));
        scanner.close();
    }
}
