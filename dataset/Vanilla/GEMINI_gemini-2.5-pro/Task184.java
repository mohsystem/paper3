import java.util.HashSet;
import java.util.Random;
import java.util.Set;

public class Task184 {
    private String name;

    private static final Set<String> usedNames = new HashSet<>();
    private static final Random random = new Random();

    public Task184() {
        this.reset();
    }

    public String getName() {
        return this.name;
    }

    public void reset() {
        if (this.name != null) {
            usedNames.remove(this.name);
        }
        
        String newName;
        do {
            newName = generateRandomName();
        } while (usedNames.contains(newName));
        
        usedNames.add(newName);
        this.name = newName;
    }

    private String generateRandomName() {
        char letter1 = (char) ('A' + random.nextInt(26));
        char letter2 = (char) ('A' + random.nextInt(26));
        int number = random.nextInt(1000);
        return String.format("%c%c%03d", letter1, letter2, number);
    }

    public static void main(String[] args) {
        System.out.println("--- Test Case 1: Create 5 robots ---");
        Task184 r1 = new Task184();
        Task184 r2 = new Task184();
        Task184 r3 = new Task184();
        Task184 r4 = new Task184();
        Task184 r5 = new Task184();

        System.out.println("Robot 1 name: " + r1.getName());
        System.out.println("Robot 2 name: " + r2.getName());
        System.out.println("Robot 3 name: " + r3.getName());
        System.out.println("Robot 4 name: " + r4.getName());
        System.out.println("Robot 5 name: " + r5.getName());

        System.out.println("\n--- Test Case 2: Reset Robot 3 ---");
        String oldNameR3 = r3.getName();
        r3.reset();
        System.out.println("Robot 3 old name: " + oldNameR3);
        System.out.println("Robot 3 new name: " + r3.getName());
        
        System.out.println("\n--- Test Case 3: Reset Robot 1 ---");
        String oldNameR1 = r1.getName();
        r1.reset();
        System.out.println("Robot 1 old name: " + oldNameR1);
        System.out.println("Robot 1 new name: " + r1.getName());

        System.out.println("\n--- Test Case 4: Show current names ---");
        System.out.println("Robot 1 name: " + r1.getName());
        System.out.println("Robot 2 name: " + r2.getName());
        System.out.println("Robot 3 name: " + r3.getName());
        System.out.println("Robot 4 name: " + r4.getName());
        System.out.println("Robot 5 name: " + r5.getName());
        
        System.out.println("\n--- Test Case 5: Create a new robot ---");
        Task184 r6 = new Task184();
        System.out.println("New Robot 6 name: " + r6.getName());
        System.out.println("Total unique names in use: " + usedNames.size());
    }
}