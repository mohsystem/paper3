import java.util.Objects;

public class Task199 {
    public static class Clock {
        private final int minutes; // minutes since 00:00, in [0, 1439]

        private Clock(int totalMinutes) {
            this.minutes = normalize(totalMinutes);
        }

        private static int normalize(int totalMinutes) {
            int m = totalMinutes % (24 * 60);
            if (m < 0) m += 24 * 60;
            return m;
        }

        // Factory method: creates a clock at given hours and minutes
        public static Clock at(int hours, int minutes) {
            return new Clock(hours * 60 + minutes);
        }

        // Add minutes, return new Clock
        public Clock add(int minutesToAdd) {
            return new Clock(this.minutes + minutesToAdd);
        }

        // Subtract minutes, return new Clock
        public Clock subtract(int minutesToSubtract) {
            return new Clock(this.minutes - minutesToSubtract);
        }

        public int hour() {
            return minutes / 60;
        }

        public int minute() {
            return minutes % 60;
        }

        @Override
        public String toString() {
            int h = hour();
            int m = minute();
            return String.format("%02d:%02d", h, m);
        }

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

    // Example helper function that takes parameters and returns a string
    public static String clockAfterAdding(int h, int m, int delta) {
        return Clock.at(h, m).add(delta).toString();
    }

    public static void main(String[] args) {
        // 5 test cases
        Clock c1 = Clock.at(10, 0).add(3);
        System.out.println("Test1 " + c1); // 10:03

        Clock c2 = Clock.at(23, 30).add(90);
        System.out.println("Test2 " + c2); // 01:00

        Clock c3 = Clock.at(0, 10).subtract(20);
        System.out.println("Test3 " + c3); // 23:50

        Clock c4a = Clock.at(15, 37);
        Clock c4b = Clock.at(14, 97); // same as 15:37
        System.out.println("Test4 equals " + c4a.equals(c4b)); // true

        Clock c5a = Clock.at(5, 32);
        Clock c5b = c5a.add(24 * 60); // wrap full day
        System.out.println("Test5 equals " + c5a.equals(c5b) + " " + c5b); // true 05:32
    }
}