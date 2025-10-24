
import java.util.Scanner;

public class Task113 {
    public static String concatenateStrings(String[] strings) {
        if (strings == null || strings.length == 0) {
            return "";
        }
        
        StringBuilder result = new StringBuilder();
        for (String str : strings) {
            if (str != null) {
                result.append(str);
            }
        }
        return result.toString();
    }
    
    public static void main(String[] args) {
        // Test case 1: Multiple strings
        String[] test1 = {"Hello", " ", "World", "!"};
        System.out.println("Test 1: " + concatenateStrings(test1));
        
        // Test case 2: Empty array
        String[] test2 = {};
        System.out.println("Test 2: " + concatenateStrings(test2));
        
        // Test case 3: Single string
        String[] test3 = {"SingleString"};
        System.out.println("Test 3: " + concatenateStrings(test3));
        
        // Test case 4: Strings with numbers and special characters
        String[] test4 = {"Java", "123", "@#$", "Test"};
        System.out.println("Test 4: " + concatenateStrings(test4));
        
        // Test case 5: Strings with null values
        String[] test5 = {"Start", null, "Middle", null, "End"};
        System.out.println("Test 5: " + concatenateStrings(test5));
        
        // Interactive user input
        Scanner scanner = new Scanner(System.in);
        System.out.println("\\nEnter number of strings to concatenate:");
        int n = scanner.nextInt();
        scanner.nextLine(); // consume newline
        
        String[] userStrings = new String[n];
        System.out.println("Enter " + n + " strings:");
        for (int i = 0; i < n; i++) {
            userStrings[i] = scanner.nextLine();
        }
        
        System.out.println("Result: " + concatenateStrings(userStrings));
        scanner.close();
    }
}
