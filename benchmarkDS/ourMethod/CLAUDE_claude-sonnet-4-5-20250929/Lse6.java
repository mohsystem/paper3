
import java.util.Scanner;

public class Lse6 {
    // Define maximum array size to prevent excessive memory usage
    private static final int MAX_ARRAY_SIZE = 100;
    
    public static String getVehicleByIndex(String[] vehicles, int index) {
        // Validate input: check if vehicles array is null
        if (vehicles == null) {
            return "Error: Invalid vehicle array";
        }
        
        // Validate input: check if index is within valid range
        // This prevents out-of-bounds access
        if (index < 0 || index >= vehicles.length) {
            return "Error: Invalid index";
        }
        
        // Return the vehicle at the specified index
        return vehicles[index];
    }
    
    public static void printAllVehicles(String[] vehicles) {
        // Validate input: check if vehicles array is null
        if (vehicles == null) {
            System.out.println("Error: Invalid vehicle array");
            return;
        }
        
        // Loop through array and print each vehicle
        // Using enhanced for loop to avoid index errors
        System.out.println("All vehicles:");
        for (String vehicle : vehicles) {
            // Validate each element is not null before printing
            if (vehicle != null) {
                System.out.println(vehicle);
            }
        }
    }
    
    public static void main(String[] args) {
        // Create array of vehicle strings
        String[] vehicles = {"Car", "Truck", "Motorcycle", "Bus", "Bicycle"};
        
        // Test cases
        System.out.println("=== Test Case 1: Valid index 0 ===");
        System.out.println(getVehicleByIndex(vehicles, 0));
        printAllVehicles(vehicles);
        
        System.out.println("\\n=== Test Case 2: Valid index 2 ===");
        System.out.println(getVehicleByIndex(vehicles, 2));
        printAllVehicles(vehicles);
        
        System.out.println("\\n=== Test Case 3: Invalid negative index ===");
        System.out.println(getVehicleByIndex(vehicles, -1));
        
        System.out.println("\\n=== Test Case 4: Invalid index too large ===");
        System.out.println(getVehicleByIndex(vehicles, 10));
        
        System.out.println("\\n=== Test Case 5: Valid index at boundary ===");
        System.out.println(getVehicleByIndex(vehicles, 4));
        
        // Interactive prompt with input validation
        Scanner scanner = null;
        try {
            scanner = new Scanner(System.in);
            System.out.println("\\n=== Interactive Mode ===");
            System.out.print("Enter vehicle index (0-" + (vehicles.length - 1) + "): ");
            
            // Validate that input is an integer
            if (scanner.hasNextInt()) {
                int userIndex = scanner.nextInt();
                String result = getVehicleByIndex(vehicles, userIndex);
                System.out.println("Result: " + result);
            } else {
                System.out.println("Error: Invalid input format");
            }
        } catch (Exception e) {
            // Generic error message to avoid leaking internal details
            System.out.println("Error: Unable to process input");
        } finally {
            // Ensure scanner is closed to prevent resource leak
            if (scanner != null) {
                scanner.close();
            }
        }
    }
}
