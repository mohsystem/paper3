
public class Task199 {
    private int hours;
    private int minutes;
    
    public Task199(int hours, int minutes) {
        if (hours < 0 || minutes < 0) {
            throw new IllegalArgumentException("Hours and minutes cannot be negative");
        }
        this.minutes = (hours * 60 + minutes) % (24 * 60);
        if (this.minutes < 0) {
            this.minutes += 24 * 60;
        }
        this.hours = this.minutes / 60;
        this.minutes = this.minutes % 60;
    }
    
    public Task199 addMinutes(int minutesToAdd) {
        int totalMinutes = this.hours * 60 + this.minutes + minutesToAdd;
        totalMinutes = totalMinutes % (24 * 60);
        if (totalMinutes < 0) {
            totalMinutes += 24 * 60;
        }
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
        return 31 * hours + minutes;
    }
    
    @Override
    public String toString() {
        return String.format("%02d:%02d", hours, minutes);
    }
    
    public static void main(String[] args) {
        // Test case 1: Basic clock creation
        Task199 clock1 = new Task199(10, 30);
        System.out.println("Test 1 - Clock1: " + clock1);
        
        // Test case 2: Adding minutes
        Task199 clock2 = clock1.addMinutes(45);
        System.out.println("Test 2 - Clock1 + 45 minutes: " + clock2);
        
        // Test case 3: Subtracting minutes
        Task199 clock3 = clock2.subtractMinutes(90);
        System.out.println("Test 3 - Clock2 - 90 minutes: " + clock3);
        
        // Test case 4: Equality check
        Task199 clock4 = new Task199(10, 30);
        System.out.println("Test 4 - Clock1 equals Clock4: " + clock1.equals(clock4));
        
        // Test case 5: Wrap around midnight
        Task199 clock5 = new Task199(23, 45);
        Task199 clock6 = clock5.addMinutes(30);
        System.out.println("Test 5 - 23:45 + 30 minutes: " + clock6);
    }
}
