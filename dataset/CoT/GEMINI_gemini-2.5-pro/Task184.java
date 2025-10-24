import java.util.HashSet;
import java.util.Random;
import java.util.Set;

class Robot {
    private static final Set<String> usedNames = new HashSet<>();
    private static final Random random = new Random();
    private String name;

    public Robot() {
        this.name = null;
    }

    public String getName() {
        if (this.name == null) {
            this.name = generateUniqueName();
        }
        return this.name;
    }

    public void reset() {
        if (this.name != null) {
            usedNames.remove(this.name);
            this.name = null;
        }
    }

    private String generateUniqueName() {
        String newName;
        do {
            newName = generateRandomName();
        } while (usedNames.contains(newName));
        usedNames.add(newName);
        return newName;
    }

    private String generateRandomName() {
        StringBuilder sb = new StringBuilder();
        // Two uppercase letters
        for (int i = 0; i < 2; i++) {
            char letter = (char) ('A' + random.nextInt(26));
            sb.append(letter);
        }
        // Three digits
        for (int i = 0; i < 3; i++) {
            sb.append(random.nextInt(10));
        }
        return sb.toString();
    }
}

public class Task184 {
    public static void main(String[] args) {
        // Test Case 1: Create a robot and get its name
        System.out.println("--- Test Case 1 ---");
        Robot r1 = new Robot();
        System.out.println("Robot 1 initial name: " + r1.getName());
        System.out.println("Robot 1 name again: " + r1.getName());

        // Test Case 2: Create another robot
        System.out.println("\n--- Test Case 2 ---");
        Robot r2 = new Robot();
        System.out.println("Robot 2 initial name: " + r2.getName());

        // Test Case 3: Reset the first robot and get a new name
        System.out.println("\n--- Test Case 3 ---");
        System.out.println("Resetting Robot 1...");
        r1.reset();
        System.out.println("Robot 1 new name: " + r1.getName());

        // Test Case 4: Create a few more robots
        System.out.println("\n--- Test Case 4 ---");
        Robot r3 = new Robot();
        Robot r4 = new Robot();
        System.out.println("Robot 3 name: " + r3.getName());
        System.out.println("Robot 4 name: " + r4.getName());

        // Test Case 5: Reset r2 and r3
        System.out.println("\n--- Test Case 5 ---");
        System.out.println("Resetting Robot 2 and 3...");
        r2.reset();
        r3.reset();
        System.out.println("Robot 2 new name: " + r2.getName());
        System.out.println("Robot 3 new name: " + r3.getName());
        System.out.println("Robot 1 current name: " + r1.getName());
        System.out.println("Robot 4 current name: " + r4.getName());
    }
}