
import java.security.SecureRandom;
import java.util.HashSet;
import java.util.Set;

class Task184 {
    private static final Set<String> usedNames = new HashSet<>();
    private static final SecureRandom random = new SecureRandom();
    private static final int MAX_NAMES = 26 * 26 * 1000; // 676,000 possible names
    private static final Object lock = new Object();
    
    private String name;
    
    public Task184() {
        this.name = generateUniqueName();
    }
    
    public String getName() {
        return this.name;
    }
    
    public void reset() {
        synchronized (lock) {
            if (this.name != null) {
                usedNames.remove(this.name);
            }
            this.name = generateUniqueName();
        }
    }
    
    private static String generateUniqueName() {
        synchronized (lock) {
            if (usedNames.size() >= MAX_NAMES) {
                throw new RuntimeException("All possible robot names have been exhausted");
            }
            
            String newName;
            do {
                newName = generateRandomName();
            } while (usedNames.contains(newName));
            
            usedNames.add(newName);
            return newName;
        }
    }
    
    private static String generateRandomName() {
        char letter1 = (char) ('A' + random.nextInt(26));
        char letter2 = (char) ('A' + random.nextInt(26));
        int digits = random.nextInt(1000);
        return String.format("%c%c%03d", letter1, letter2, digits);
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Create robot and get name");
        Task184 robot1 = new Task184();
        String name1 = robot1.getName();
        System.out.println("Robot 1 name: " + name1);
        System.out.println("Name matches pattern: " + name1.matches("[A-Z]{2}\\\\d{3}"));
        
        System.out.println("\\nTest Case 2: Create another robot with different name");
        Task184 robot2 = new Task184();
        String name2 = robot2.getName();
        System.out.println("Robot 2 name: " + name2);
        System.out.println("Names are different: " + !name1.equals(name2));
        
        System.out.println("\\nTest Case 3: Reset robot and verify new name");
        String oldName = robot1.getName();
        robot1.reset();
        String newName = robot1.getName();
        System.out.println("Old name: " + oldName);
        System.out.println("New name: " + newName);
        System.out.println("Names are different: " + !oldName.equals(newName));
        
        System.out.println("\\nTest Case 4: Create multiple robots");
        Task184 robot3 = new Task184();
        Task184 robot4 = new Task184();
        Task184 robot5 = new Task184();
        System.out.println("Robot 3: " + robot3.getName());
        System.out.println("Robot 4: " + robot4.getName());
        System.out.println("Robot 5: " + robot5.getName());
        
        System.out.println("\\nTest Case 5: Verify uniqueness after resets");
        robot2.reset();
        robot3.reset();
        Set<String> allNames = new HashSet<>();
        allNames.add(robot1.getName());
        allNames.add(robot2.getName());
        allNames.add(robot3.getName());
        allNames.add(robot4.getName());
        allNames.add(robot5.getName());
        System.out.println("All 5 names are unique: " + (allNames.size() == 5));
    }
}
