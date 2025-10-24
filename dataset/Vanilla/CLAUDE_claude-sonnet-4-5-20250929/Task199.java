
public class Task199 {
    private int hours;
    private int minutes;
    
    public Task199(int hours, int minutes) {
        this.minutes = (hours * 60 + minutes) % (24 * 60);
        if (this.minutes < 0) {
            this.minutes += 24 * 60;
        }
        this.hours = this.minutes / 60;
        this.minutes = this.minutes % 60;
    }
    
    public Task199 addMinutes(int minutesToAdd) {
        int totalMinutes = this.hours * 60 + this.minutes + minutesToAdd;
        return new Task199(0, totalMinutes);
    }
    
    public Task199 subtractMinutes(int minutesToSubtract) {
        return addMinutes(-minutesToSubtract);
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
        return 60 * hours + minutes;
    }
    
    @Override
    public String toString() {
        return String.format("%02d:%02d", hours, minutes);
    }
    
    public static void main(String[] args) {
        // Test case 1: Create a clock and display it
        Task199 clock1 = new Task199(10, 30);
        System.out.println("Test 1 - Clock at 10:30: " + clock1);
        
        // Test case 2: Add minutes
        Task199 clock2 = clock1.addMinutes(45);
        System.out.println("Test 2 - Add 45 minutes to 10:30: " + clock2);
        
        // Test case 3: Subtract minutes
        Task199 clock3 = clock1.subtractMinutes(90);
        System.out.println("Test 3 - Subtract 90 minutes from 10:30: " + clock3);
        
        // Test case 4: Wrap around midnight (add)
        Task199 clock4 = new Task199(23, 45);
        Task199 clock5 = clock4.addMinutes(30);
        System.out.println("Test 4 - Add 30 minutes to 23:45: " + clock5);
        
        // Test case 5: Equality check
        Task199 clock6 = new Task199(11, 15);
        Task199 clock7 = new Task199(11, 15);
        Task199 clock8 = new Task199(11, 16);
        System.out.println("Test 5 - 11:15 equals 11:15: " + clock6.equals(clock7));
        System.out.println("Test 5 - 11:15 equals 11:16: " + clock6.equals(clock8));
    }
}
