import java.util.Objects;

public class Task199 {
    // Secure, immutable Clock handling hours and minutes without dates.
    public static final class Clock {
        private final int minutes; // total minutes normalized to [0, 1439]

        private Clock(long totalMinutes) {
            this.minutes = (int)Math.floorMod(totalMinutes, 1440L);
        }

        // Factory method to create a clock from hour and minute
        public static Clock of(int hour, int minute) {
            long total = (long)hour * 60L + (long)minute;
            return new Clock(total);
        }

        // Add minutes, returning a new Clock
        public Clock plus(int minutesToAdd) {
            return new Clock((long)this.minutes + (long)minutesToAdd);
        }

        // Subtract minutes, returning a new Clock
        public Clock minus(int minutesToSub) {
            return new Clock((long)this.minutes - (long)minutesToSub);
        }

        // String representation in HH:MM
        @Override
        public String toString() {
            int h = minutes / 60;
            int m = minutes % 60;
            return String.format("%02d:%02d", h, m);
        }

        // Equality based on normalized time
        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (!(o instanceof Clock)) return false;
            Clock clock = (Clock) o;
            return minutes == clock.minutes;
        }

        @Override
        public int hashCode() {
            return Objects.hash(minutes);
        }
    }

    // Test harness with 5 test cases
    public static void main(String[] args) {
        // Test 1: Basic creation and toString
        Clock c1 = Clock.of(10, 37);
        System.out.println("Test1: " + c1); // Expected 10:37

        // Test 2: Add minutes with hour rollover
        Clock c2 = Clock.of(10, 0).plus(3);
        System.out.println("Test2: " + c2); // Expected 10:03

        // Test 3: Add minutes across midnight
        Clock c3 = Clock.of(23, 59).plus(2);
        System.out.println("Test3: " + c3); // Expected 00:01

        // Test 4: Subtract minutes across midnight
        Clock c4 = Clock.of(0, 5).minus(10);
        System.out.println("Test4: " + c4); // Expected 23:55

        // Test 5: Equality of clocks representing same time
        Clock c5a = Clock.of(24, 0);
        Clock c5b = Clock.of(48, 0);
        System.out.println("Test5: " + c5a + " == " + c5b + " -> " + c5a.equals(c5b)); // Expected true
    }
}