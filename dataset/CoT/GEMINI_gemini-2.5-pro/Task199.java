import java.util.Objects;

public class Task199 {

    static class Clock {
        private final int hours;
        private final int minutes;
        private static final int MINUTES_IN_DAY = 24 * 60;

        public Clock(int hours, int minutes) {
            int totalMinutes = hours * 60 + minutes;
            // The formula (a % n + n) % n handles negative results correctly.
            totalMinutes = (totalMinutes % MINUTES_IN_DAY + MINUTES_IN_DAY) % MINUTES_IN_DAY;
            this.hours = totalMinutes / 60;
            this.minutes = totalMinutes % 60;
        }

        public Clock add(int minutesToAdd) {
            return new Clock(this.hours, this.minutes + minutesToAdd);
        }

        public Clock subtract(int minutesToSubtract) {
            return new Clock(this.hours, this.minutes - minutesToSubtract);
        }

        @Override
        public String toString() {
            return String.format("%02d:%02d", this.hours, this.minutes);
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (o == null || getClass() != o.getClass()) return false;
            Clock clock = (Clock) o;
            return hours == clock.hours && minutes == clock.minutes;
        }

        @Override
        public int hashCode() {
            return Objects.hash(hours, minutes);
        }
    }

    public static void main(String[] args) {
        // Test Case 1: Simple creation and toString
        Clock clock1 = new Clock(8, 30);
        System.out.println("Test Case 1: Simple creation");
        System.out.println("Clock(8, 30) -> " + clock1); // Expected: 08:30
        System.out.println("--------------------");

        // Test Case 2: Adding minutes without hour rollover
        Clock clock2 = new Clock(10, 0).add(25);
        System.out.println("Test Case 2: Add minutes (no rollover)");
        System.out.println("Clock(10, 0).add(25) -> " + clock2); // Expected: 10:25
        System.out.println("--------------------");

        // Test Case 3: Adding minutes with hour and day rollover
        Clock clock3 = new Clock(23, 45).add(30);
        System.out.println("Test Case 3: Add minutes (with rollover)");
        System.out.println("Clock(23, 45).add(30) -> " + clock3); // Expected: 00:15
        System.out.println("--------------------");

        // Test Case 4: Subtracting minutes with hour and day rollover
        Clock clock4 = new Clock(0, 10).subtract(30);
        System.out.println("Test Case 4: Subtract minutes (with rollover)");
        System.out.println("Clock(0, 10).subtract(30) -> " + clock4); // Expected: 23:40
        System.out.println("--------------------");

        // Test Case 5: Equality and normalization
        Clock clock5a = new Clock(15, 30);
        Clock clock5b = new Clock(14, 90); // Normalizes to 15:30
        Clock clock5c = new Clock(16, -30); // Normalizes to 15:30
        Clock clock5d = new Clock(39, 30); // Normalizes to 15:30 (39h = 24h + 15h)
        Clock clock5e = new Clock(15, 31); // Not equal
        System.out.println("Test Case 5: Equality and normalization");
        System.out.println("Clock(15, 30) equals Clock(14, 90)? " + clock5a.equals(clock5b)); // Expected: true
        System.out.println("Clock(15, 30) equals Clock(16, -30)? " + clock5a.equals(clock5c)); // Expected: true
        System.out.println("Clock(15, 30) equals Clock(39, 30)? " + clock5a.equals(clock5d)); // Expected: true
        System.out.println("Clock(15, 30) equals Clock(15, 31)? " + clock5a.equals(clock5e)); // Expected: false
        System.out.println("--------------------");
    }
}