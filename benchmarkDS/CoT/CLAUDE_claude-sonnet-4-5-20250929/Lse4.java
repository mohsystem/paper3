
import java.util.Scanner;

public class Lse4 {
    private static final String[] vehicles = {
        "Toyota Camry",
        "Honda Civic",
        "Ford F-150",
        "Tesla Model 3",
        "Chevrolet Silverado"
    };
    
    public static String getVehicleAtIndex(int index) {
        // Validate index bounds
        if (index < 0 || index >= vehicles.length) {
            return "Error: Index out of bounds. Valid range: 0 to " + (vehicles.length - 1);
        }
        return vehicles[index];
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("=== Test Cases ===");
        System.out.println("Test 1 (index 0): " + getVehicleAtIndex(0));
        System.out.println("Test 2 (index 2): " + getVehicleAtIndex(2));
        System.out.println("Test 3 (index 4): " + getVehicleAtIndex(4));
        System.out.println("Test 4 (index -1): " + getVehicleAtIndex(-1));
        System.out.println("Test 5 (index 10): " + getVehicleAtIndex(10));
        
        // Interactive mode
        System.out.println("\\n=== Interactive Mode ===");
        Scanner scanner = new Scanner(System.in);
        
        try {
            System.out.print("Enter an index of a vehicle (0-" + (vehicles.length - 1) + "): ");
            String input = scanner.nextLine().trim();
            
            // Validate input is not empty
            if (input.isEmpty()) {
                System.out.println("Error: Input cannot be empty");
                return;
            }
            
            // Parse and validate integer
            int index = Integer.parseInt(input);
            System.out.println("Result: " + getVehicleAtIndex(index));
            
        } catch (NumberFormatException e) {
            System.out.println("Error: Invalid input. Please enter a valid integer.");
        } finally {
            scanner.close();
        }
    }
}
