public class Task199 {
    private static final int MINUTES_PER_HOUR = 60;
    private static final int HOURS_PER_DAY = 24;
    private static final int MINUTES_PER_DAY = MINUTES_PER_HOUR * HOURS_PER_DAY; // 1440

    private int totalMinutes;

    public Task199(int hour, int minute) {
        // Normalize the time
        int total = hour * MINUTES_PER_HOUR + minute;
        // The modulo operator in Java can return a negative result.
        // We add MINUTES_PER_DAY and take the modulo again to ensure a positive result.
        this.totalMinutes = (total % MINUTES_PER_DAY + MINUTES_PER_DAY) % MINUTES_PER_DAY;
    }

    public void add(int minutes) {
        this.totalMinutes = (this.totalMinutes + minutes % MINUTES_PER_DAY + MINUTES_PER_DAY) % MINUTES_PER_DAY;
    }

    public void subtract(int minutes) {
        // Subtracting is the same as adding a negative number
        add(-minutes);
    }

    @Override
    public String toString() {
        int hour = totalMinutes / MINUTES_PER_HOUR;
        int minute = totalMinutes % MINUTES_PER_HOUR;
        return String.format("%02d:%02d", hour, minute);
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) return true;
        if (obj == null || getClass() != obj.getClass()) return false;
        Task199 clock = (Task199) obj;
        return totalMinutes == clock.totalMinutes;
    }

    @Override
    public int hashCode() {
        // It's good practice to override hashCode when you override equals
        return Integer.hashCode(totalMinutes);
    }

    public static void main(String[] args) {
        // Test Case 1: Create a clock and test toString
        Task199 clock1 = new Task199(10, 30);
        System.out.println("Test Case 1: Create clock (10:30) -> " + clock1);

        // Test Case 2: Add minutes without day rollover
        clock1.add(20);
        System.out.println("Test Case 2: Add 20 mins to 10:30 -> " + clock1);

        // Test Case 3: Add minutes with day rollover
        Task199 clock2 = new Task199(23, 50);
        clock2.add(20);
        System.out.println("Test Case 3: Add 20 mins to 23:50 -> " + clock2);

        // Test Case 4: Subtract minutes with day rollover
        Task199 clock3 = new Task199(0, 10);
        clock3.subtract(20);
        System.out.println("Test Case 4: Subtract 20 mins from 00:10 -> " + clock3);
        
        // Test Case 5: Equality
        Task199 clockA = new Task199(14, 30);
        Task199 clockB = new Task199(14, 30);
        Task199 clockC = new Task199(10, 0);
        Task199 clockD = new Task199(38, 30); // equivalent to 14:30
        System.out.println("Test Case 5: Equality Check");
        System.out.println("  14:30 equals 14:30 -> " + clockA.equals(clockB));
        System.out.println("  14:30 equals 10:00 -> " + clockA.equals(clockC));
        System.out.println("  14:30 equals normalized 38:30 -> " + clockA.equals(clockD));
    }
}