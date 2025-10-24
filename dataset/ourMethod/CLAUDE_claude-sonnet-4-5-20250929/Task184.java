
import java.security.SecureRandom;
import java.util.HashSet;
import java.util.Set;

public class Task184 {
    private static final Set<String> usedNames = new HashSet<>();
    private static final SecureRandom secureRandom = new SecureRandom();
    private static final int MAX_ATTEMPTS = 10000;
    
    private String name;
    
    public Task184() {
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
            synchronized (usedNames) {
                usedNames.remove(this.name);
            }
            this.name = null;
        }
    }
    
    private static String generateUniqueName() {
        synchronized (usedNames) {
            for (int attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
                String candidate = generateRandomName();
                if (usedNames.add(candidate)) {
                    return candidate;
                }
            }
            throw new RuntimeException("Failed to generate unique name after " + MAX_ATTEMPTS + " attempts");
        }
    }
    
    private static String generateRandomName() {
        char letter1 = (char) ('A' + secureRandom.nextInt(26));
        char letter2 = (char) ('A' + secureRandom.nextInt(26));
        int digit1 = secureRandom.nextInt(10);
        int digit2 = secureRandom.nextInt(10);
        int digit3 = secureRandom.nextInt(10);
        return String.format("%c%c%d%d%d", letter1, letter2, digit1, digit2, digit3);
    }
    
    public static void main(String[] args) {
        Task184 robot1 = new Task184();
        System.out.println("Robot 1 name: " + robot1.getName());
        System.out.println("Robot 1 name again: " + robot1.getName());
        
        Task184 robot2 = new Task184();
        System.out.println("Robot 2 name: " + robot2.getName());
        
        robot1.reset();
        System.out.println("Robot 1 after reset: " + robot1.getName());
        
        Task184 robot3 = new Task184();
        System.out.println("Robot 3 name: " + robot3.getName());
        
        robot2.reset();
        System.out.println("Robot 2 after reset: " + robot2.getName());
    }
}
