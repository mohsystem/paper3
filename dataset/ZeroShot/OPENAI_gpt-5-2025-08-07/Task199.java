import java.util.Locale;

public class Task199 {

    public static final class Clock {
        private static final int MINUTES_PER_DAY = 24 * 60;
        private final int minutes; // minutes since midnight, 0..1439

        private Clock(int normalizedMinutes) {
            this.minutes = normalizedMinutes;
        }

        public static Clock at(int hour, int minute) {
            long total = (long) hour * 60L + (long) minute;
            int norm = (int) Math.floorMod(total, (long) MINUTES_PER_DAY);
            return new Clock(norm);
        }

        public Clock add(int minutesToAdd) {
            long total = (long) this.minutes + (long) minutesToAdd;
            int norm = (int) Math.floorMod(total, (long) MINUTES_PER_DAY);
            return new Clock(norm);
        }

        public Clock subtract(int minutesToSubtract) {
            return add(-minutesToSubtract);
        }

        public int hour() {
            return minutes / 60;
        }

        public int minute() {
            return minutes % 60;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (!(obj instanceof Clock)) return false;
            Clock other = (Clock) obj;
            return this.minutes == other.minutes;
        }

        @Override
        public int hashCode() {
            return Integer.hashCode(minutes);
        }

        @Override
        public String toString() {
            return String.format(Locale.ROOT, "%02d:%02d", hour(), minute());
        }
    }

    // Example API functions that accept inputs and return outputs
    public static Clock createClock(int hour, int minute) {
        return Clock.at(hour, minute);
    }

    public static Clock addMinutes(Clock clock, int minutes) {
        return clock.add(minutes);
    }

    public static Clock subtractMinutes(Clock clock, int minutes) {
        return clock.subtract(minutes);
    }

    public static boolean clocksEqual(Clock a, Clock b) {
        return a.equals(b);
    }

    public static String clockToString(Clock c) {
        return c.toString();
    }

    public static void main(String[] args) {
        // 5 test cases
        Clock t1 = createClock(10, 0);
        t1 = addMinutes(t1, 3);
        System.out.println("Test1: " + clockToString(t1)); // 10:03

        Clock t2 = createClock(23, 59);
        t2 = addMinutes(t2, 2);
        System.out.println("Test2: " + clockToString(t2)); // 00:01

        Clock t3 = createClock(0, 3);
        t3 = subtractMinutes(t3, 4);
        System.out.println("Test3: " + clockToString(t3)); // 23:59

        Clock t4a = createClock(25, 0);
        Clock t4b = createClock(1, 0);
        System.out.println("Test4: equal=" + clocksEqual(t4a, t4b)); // true

        Clock t5 = createClock(2, 30);
        t5 = addMinutes(t5, -1500); // -25 hours
        System.out.println("Test5: " + clockToString(t5) + " equal 01:30? " + clocksEqual(t5, createClock(1, 30))); // 01:30 true
    }
}