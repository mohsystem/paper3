import java.util.Objects;

public class Task199 {

    public static final class Clock {
        private final int hour;
        private final int minute;

        private Clock(int hour, int minute) {
            this.hour = hour;
            this.minute = minute;
        }

        public static Clock of(int hours, int minutes) {
            long total = (long) hours * 60L + (long) minutes;
            long mod = Math.floorMod(total, 1440L);
            int h = (int) (mod / 60L);
            int m = (int) (mod % 60L);
            return new Clock(h, m);
        }

        public Clock addMinutes(int delta) {
            return Clock.of(this.hour, this.minute + delta);
        }

        public Clock subtractMinutes(int delta) {
            return Clock.of(this.hour, this.minute - delta);
        }

        public int getHour() {
            return hour;
        }

        public int getMinute() {
            return minute;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (!(obj instanceof Clock)) return false;
            Clock other = (Clock) obj;
            return this.hour == other.hour && this.minute == other.minute;
        }

        @Override
        public int hashCode() {
            return Objects.hash(hour, minute);
        }

        @Override
        public String toString() {
            return String.format("%02d:%02d", hour, minute);
        }
    }

    public static void main(String[] args) {
        // Test case 1: 10:00 + 3 minutes = 10:03
        Clock t1 = Clock.of(10, 0).addMinutes(3);
        System.out.println("Test1 " + t1.toString());

        // Test case 2: 10:00 + 61 minutes = 11:01
        Clock t2 = Clock.of(10, 0).addMinutes(61);
        System.out.println("Test2 " + t2.toString());

        // Test case 3: 23:59 + 2 minutes = 00:01
        Clock t3 = Clock.of(23, 59).addMinutes(2);
        System.out.println("Test3 " + t3.toString());

        // Test case 4: 00:30 - 90 minutes = 23:00
        Clock t4 = Clock.of(0, 30).subtractMinutes(90);
        System.out.println("Test4 " + t4.toString());

        // Test case 5: equality: 25:00 == 01:00
        Clock a = Clock.of(25, 0);
        Clock b = Clock.of(1, 0);
        System.out.println("Test5 " + a.toString() + " == " + b.toString() + " -> " + a.equals(b));
    }
}