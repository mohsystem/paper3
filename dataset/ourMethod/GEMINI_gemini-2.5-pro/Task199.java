import java.util.Objects;

public class Task199 {

    public static final class Clock {
        private static final int MINUTES_PER_HOUR = 60;
        private static final int HOURS_PER_DAY = 24;
        private static final int MINUTES_PER_DAY = HOURS_PER_DAY * MINUTES_PER_HOUR;

        private final int totalMinutes;

        public Clock(int hour, int minute) {
            int effectiveTotalMinutes = hour * MINUTES_PER_HOUR + minute;
            // The formula (a % n + n) % n handles negative results correctly,
            // ensuring the value is always in the range [0, n-1].
            this.totalMinutes = (effectiveTotalMinutes % MINUTES_PER_DAY + MINUTES_PER_DAY) % MINUTES_PER_DAY;
        }
        
        public Clock add(int minutes) {
            // Create a new Clock object by passing the new total minutes
            // to the constructor, which will handle normalization.
            // Hour is set to 0 as we are working with total minutes.
            return new Clock(0, this.totalMinutes + minutes);
        }

        public Clock subtract(int minutes) {
            return new Clock(0, this.totalMinutes - minutes);
        }

        @Override
        public String toString() {
            int hour = totalMinutes / MINUTES_PER_HOUR;
            int minute = totalMinutes % MINUTES_PER_HOUR;
            return String.format("%02d:%02d", hour, minute);
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (o == null || getClass() != o.getClass()) return false;
            Clock clock = (Clock) o;
            return totalMinutes == clock.totalMinutes;
        }

        @Override
        public int hashCode() {
            return Objects.hash(totalMinutes);
        }
    }

    public static void main(String[] args) {
        // Test Case 1: Create a clock and check its string representation
        Clock clock1 = new Clock(10, 30);
        System.out.println("Test 1: Create 10:30 -> " + clock1);

        // Test Case 2: Add minutes
        Clock clock2 = clock1.add(70);
        System.out.println("Test 2: 10:30 + 70 min -> " + clock2);

        // Test Case 3: Subtract minutes, wrapping backwards
        Clock clock3 = new Clock(0, 15).subtract(30);
        System.out.println("Test 3: 00:15 - 30 min -> " + clock3);

        // Test Case 4: Equality check with different but equivalent inputs
        Clock clock4a = new Clock(14, 0);
        Clock clock4b = new Clock(13, 60);
        System.out.println("Test 4: 14:00 equals 13:60 -> " + clock4a.equals(clock4b));

        // Test Case 5: Add minutes, wrapping forwards past midnight
        Clock clock5 = new Clock(23, 50).add(20);
        System.out.println("Test 5: 23:50 + 20 min -> " + clock5);
    }
}