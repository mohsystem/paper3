
import java.util.Scanner;
import java.util.InputMismatchException;

public class Lse4 {
    // Predefined array of vehicles
    private static final String[] VEHICLES = {"Car", "Truck", "Motorcycle", "Bus", "Van"};
    
    public static String getVehicleAtIndex(int index) {
        // Input validation: check if index is within valid bounds
        // This prevents array index out of bounds vulnerabilities
        if (index < 0 || index >= VEHICLES.length) {
            return "Error: Invalid index";
        }
        
        // Safe array access after bounds validation
        return VEHICLES[index];
    }
    
    public static void main(String[] args) {
        Scanner scanner = null;
        try {
            scanner = new Scanner(System.in);
            
            // Prompt user for input
            System.out.print("Enter an index of a vehicle (0-" + (VEHICLES.length - 1) + "): ");
            
            // Input validation: ensure input is an integer
            // This prevents type confusion and injection attacks
            if (!scanner.hasNextInt()) {
                System.out.println("Error: Invalid input format");
                return;
            }
            
            int index = scanner.nextInt();
            
            // Get vehicle using validated index
            String result = getVehicleAtIndex(index);
            System.out.println(result);
            
        } catch (InputMismatchException e) {
            // Handle input type mismatch - fail closed with generic message
            System.out.println("Error: Invalid input");
        } catch (Exception e) {
            // Catch any unexpected exceptions - fail closed without leaking details
            System.out.println("Error: An error occurred");
        } finally {
            // Resource cleanup
            if (scanner != null) {
                scanner.close();
            }
        }
        
        // Test cases
        System.out.println("\\n--- Test Cases ---");
        System.out.println("Test 1 (index 0): " + getVehicleAtIndex(0));
        System.out.println("Test 2 (index 2): " + getVehicleAtIndex(2));
        System.out.println("Test 3 (index 4): " + getVehicleAtIndex(4));
        System.out.println("Test 4 (index -1): " + getVehicleAtIndex(-1));
        System.out.println("Test 5 (index 10): " + getVehicleAtIndex(10));
    }
}
