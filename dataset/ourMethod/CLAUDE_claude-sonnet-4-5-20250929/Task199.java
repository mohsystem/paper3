
import java.util.Objects;

public class Task199 {
    private final int minutes;
    private static final int MINUTES_PER_HOUR = 60;
    private static final int HOURS_PER_DAY = 24;
    private static final int MINUTES_PER_DAY = MINUTES_PER_HOUR * HOURS_PER_DAY;

    public Task199(int hours, int minutes) {
        if (hours < 0 || hours > 23) {
            throw new IllegalArgumentException("Hours must be between 0 and 23");
        }
        if (minutes < 0 || minutes > 59) {
            throw new IllegalArgumentException("Minutes must be between 0 and 59");
        }
        this.minutes = (hours * MINUTES_PER_HOUR + minutes) % MINUTES_PER_DAY;
    }

    private Task199(int totalMinutes) {
        this.minutes = ((totalMinutes % MINUTES_PER_DAY) + MINUTES_PER_DAY) % MINUTES_PER_DAY;
    }

    public Task199 addMinutes(int minutesToAdd) {
        if (minutesToAdd < Integer.MIN_VALUE / 2 || minutesToAdd > Integer.MAX_VALUE / 2) {
            throw new IllegalArgumentException("Minutes to add out of safe range");
        }
        return new Task199(this.minutes + minutesToAdd);
    }

    public Task199 subtractMinutes(int minutesToSubtract) {
        if (minutesToSubtract < Integer.MIN_VALUE / 2 || minutesToSubtract > Integer.MAX_VALUE / 2) {
            throw new IllegalArgumentException("Minutes to subtract out of safe range");
        }
        return new Task199(this.minutes - minutesToSubtract);
    }

    public int getHours() {
        return minutes / MINUTES_PER_HOUR;
    }

    public int getMinutes() {
        return minutes % MINUTES_PER_HOUR;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) return true;
        if (obj == null || getClass() != obj.getClass()) return false;
        Task199 clock = (Task199) obj;
        return minutes == clock.minutes;
    }

    @Override
    public int hashCode() {
        return Objects.hash(minutes);
    }

    @Override
    public String toString() {
        return String.format("%02d:%02d", getHours(), getMinutes());
    }

    public static void main(String[] args) {
        // Test case 1: Basic clock creation
        Task199 clock1 = new Task199(10, 30);
        System.out.println("Test 1 - Clock created: " + clock1);

        // Test case 2: Add minutes
        Task199 clock2 = clock1.addMinutes(45);
        System.out.println("Test 2 - Add 45 minutes to 10:30: " + clock2);

        // Test case 3: Subtract minutes
        Task199 clock3 = new Task199(12, 15);
        Task199 clock4 = clock3.subtractMinutes(30);
        System.out.println("Test 3 - Subtract 30 minutes from 12:15: " + clock4);

        // Test case 4: Equality check
        Task199 clock5 = new Task199(8, 20);
        Task199 clock6 = new Task199(8, 20);
        System.out.println("Test 4 - Clocks equal (8:20 == 8:20): " + clock5.equals(clock6));

        // Test case 5: Wrap around midnight
        Task199 clock7 = new Task199(23, 45);
        Task199 clock8 = clock7.addMinutes(30);
        System.out.println("Test 5 - Add 30 minutes to 23:45 (wrap around): " + clock8);
    }
}
