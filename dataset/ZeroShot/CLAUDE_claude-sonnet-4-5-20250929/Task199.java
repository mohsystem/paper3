
public class Task199 {
    private int hours;
    private int minutes;
    
    public Task199(int hours, int minutes) {
        int totalMinutes = hours * 60 + minutes;
        totalMinutes = totalMinutes % (24 * 60);
        if (totalMinutes < 0) {
            totalMinutes += 24 * 60;
        }
        this.hours = totalMinutes / 60;
        this.minutes = totalMinutes % 60;
    }
    
    public void addMinutes(int minutesToAdd) {
        int totalMinutes = this.hours * 60 + this.minutes + minutesToAdd;
        totalMinutes = totalMinutes % (24 * 60);
        if (totalMinutes < 0) {
            totalMinutes += 24 * 60;
        }
        this.hours = totalMinutes / 60;
        this.minutes = totalMinutes % 60;
    }
    
    public void subtractMinutes(int minutesToSubtract) {
        addMinutes(-minutesToSubtract);
    }
    
    @Override
    public boolean equals(Object obj) {
        if (this == obj) return true;
        if (obj == null || getClass() != obj.getClass()) return false;
        Task199 clock = (Task199) obj;
        return hours == clock.hours && minutes == clock.minutes;
    }
    
    @Override
    public int hashCode() {
        return hours * 60 + minutes;
    }
    
    @Override
    public String toString() {
        return String.format("%02d:%02d", hours, minutes);
    }
    
    public static void main(String[] args) {
        // Test case 1: Basic clock creation
        Task199 clock1 = new Task199(10, 30);
        System.out.println("Test 1 - Clock creation: " + clock1);
        
        // Test case 2: Add minutes
        Task199 clock2 = new Task199(10, 30);
        clock2.addMinutes(90);
        System.out.println("Test 2 - Add 90 minutes to 10:30: " + clock2);
        
        // Test case 3: Subtract minutes
        Task199 clock3 = new Task199(10, 30);
        clock3.subtractMinutes(45);
        System.out.println("Test 3 - Subtract 45 minutes from 10:30: " + clock3);
        
        // Test case 4: Clock equality
        Task199 clock4a = new Task199(15, 45);
        Task199 clock4b = new Task199(15, 45);
        System.out.println("Test 4 - Clock equality (15:45 == 15:45): " + clock4a.equals(clock4b));
        
        // Test case 5: Wrap around midnight
        Task199 clock5 = new Task199(23, 30);
        clock5.addMinutes(45);
        System.out.println("Test 5 - Add 45 minutes to 23:30 (wrap around): " + clock5);
    }
}
