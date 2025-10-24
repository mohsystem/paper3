import java.security.SecureRandom;
import java.util.Collections;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

class RobotFactory {
    // Use a thread-safe set to store names in use.
    private static final Set<String> namesInUse = Collections.newSetFromMap(new ConcurrentHashMap<>());
    private static final SecureRandom random = new SecureRandom();
    private static final String ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    private static final String DIGITS = "0123456789";
    // Total possible names: 26 * 26 * 10 * 10 * 10 = 676,000
    private static final int MAX_NAMES = 26 * 26 * 1000;


    public static String generateUniqueName() {
        if (namesInUse.size() >= MAX_NAMES) {
            // Or throw a custom exception
            return null; 
        }

        String name;
        // Loop until a unique name is found. This is generally fast unless the set is nearly full.
        // The add() operation on a ConcurrentHashMap-backed set is atomic.
        do {
            StringBuilder sb = new StringBuilder(5);
            // Two uppercase letters
            sb.append(ALPHABET.charAt(random.nextInt(ALPHABET.length())));
            sb.append(ALPHABET.charAt(random.nextInt(ALPHABET.length())));
            // Three digits
            sb.append(DIGITS.charAt(random.nextInt(DIGITS.length())));
            sb.append(DIGITS.charAt(random.nextInt(DIGITS.length())));
            sb.append(DIGITS.charAt(random.nextInt(DIGITS.length())));
            name = sb.toString();
        } while (!namesInUse.add(name)); 

        return name;
    }

    public static void releaseName(String name) {
        if (name != null) {
            namesInUse.remove(name);
        }
    }
}

class Robot {
    private String name;

    public Robot() {
        this.name = null; // No name when it comes off the factory floor
    }

    public String getName() {
        if (this.name == null) {
            this.name = RobotFactory.generateUniqueName();
        }
        return this.name;
    }

    public void reset() {
        if (this.name != null) {
            RobotFactory.releaseName(this.name);
            this.name = null;
        }
    }
}

public class Task184 {
    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");

        // Test Case 1: Create a robot and get its name
        Robot robot1 = new Robot();
        System.out.println("Robot 1 initial name: " + robot1.getName());

        // Test Case 2: Create another robot and get its name
        Robot robot2 = new Robot();
        System.out.println("Robot 2 initial name: " + robot2.getName());

        // Test Case 3: Reset the first robot and get a new name
        System.out.println("Resetting Robot 1...");
        String oldName1 = robot1.getName();
        robot1.reset();
        System.out.println("Robot 1 old name was: " + oldName1);
        System.out.println("Robot 1 new name is: " + robot1.getName());

        // Test Case 4: Create more robots
        Robot robot3 = new Robot();
        Robot robot4 = new Robot();
        System.out.println("Robot 3 name: " + robot3.getName());
        System.out.println("Robot 4 name: " + robot4.getName());

        // Test Case 5: Get the name again, should be the same
        System.out.println("Robot 4 name (again): " + robot4.getName());
        
        System.out.println("---------------------\n");
    }
}