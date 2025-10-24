public class Task199 {

    static class Clock {
        private int totalMinutes;
        private static final int MINUTES_IN_DAY = 24 * 60;

        public Clock(int hours, int minutes) {
            int initialTotalMinutes = hours * 60 + minutes;
            // Handle negative results of % correctly for wraparound
            this.totalMinutes = (initialTotalMinutes % MINUTES_IN_DAY + MINUTES_IN_DAY) % MINUTES_IN_DAY;
        }

        public void add(int minutes) {
            this.totalMinutes = (this.totalMinutes + minutes % MINUTES_IN_DAY + MINUTES_IN_DAY) % MINUTES_IN_DAY;
        }

        public void subtract(int minutes) {
            add(-minutes);
        }

        @Override
        public String toString() {
            int hours = totalMinutes / 60;
            int minutes = totalMinutes % 60;
            return String.format("%02d:%02d", hours, minutes);
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (obj == null || getClass() != obj.getClass()) return false;
            Clock clock = (Clock) obj;
            // Normalize the other clock's time for comparison as well
            return this.totalMinutes == clock.totalMinutes;
        }

        @Override
        public int hashCode() {
            return Integer.hashCode(totalMinutes);
        }
    }

    public static void main(String[] args) {
        System.out.println("Java Test Cases:");
        
        // Test 1: Creation and ToString
        Clock c1 = new Clock(8, 0);
        System.out.println("Test 1: Create 08:00 -> " + c1.toString());

        // Test 2: Equality
        Clock c2 = new Clock(8, 0);
        Clock c3 = new Clock(9, 0);
        System.out.println("Test 2: 08:00 == 08:00 -> " + c1.equals(c2));
        System.out.println("Test 2: 08:00 == 09:00 -> " + c1.equals(c3));

        // Test 3: Addition
        Clock c4 = new Clock(10, 0);
        c4.add(63);
        System.out.println("Test 3: 10:00 + 63 mins -> " + c4.toString());

        // Test 4: Subtraction with rollover
        Clock c5 = new Clock(0, 30);
        c5.subtract(60);
        System.out.println("Test 4: 00:30 - 60 mins -> " + c5.toString());

        // Test 5: Complex Addition with rollover
        Clock c6 = new Clock(23, 45);
        c6.add(180);
        System.out.println("Test 5: 23:45 + 180 mins -> " + c6.toString());
    }
}