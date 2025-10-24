import java.util.Objects;

public class Task115 {

    /**
     * A class that simulates a C-style struct with bit fields using bitwise operations.
     * It packs multiple boolean and small integer values into a single integer field.
     *
     * Bit layout (from right to left, LSB is bit 0):
     * - Bit 0: isActive (1 bit)
     * - Bit 1: isReady (1 bit)
     * - Bits 2-3: errorState (2 bits, values 0-3)
     * - Bits 4-6: mode (3 bits, values 0-7)
     */
    static class HardwareFlags {
        private int flags;

        // Masks and shifts for each field
        private static final int IS_ACTIVE_MASK = 1 << 0; // 0b0000001
        private static final int IS_READY_MASK = 1 << 1;  // 0b0000010

        private static final int ERROR_STATE_SHIFT = 2;
        private static final int ERROR_STATE_MASK = 0b11 << ERROR_STATE_SHIFT; // 0b0001100

        private static final int MODE_SHIFT = 4;
        private static final int MODE_MASK = 0b111 << MODE_SHIFT; // 0b1110000

        public HardwareFlags() {
            this.flags = 0;
        }

        // Getter and Setter for isActive (1 bit)
        public boolean isActive() {
            return (flags & IS_ACTIVE_MASK) != 0;
        }

        public void setActive(boolean active) {
            if (active) {
                flags |= IS_ACTIVE_MASK;
            } else {
                flags &= ~IS_ACTIVE_MASK;
            }
        }

        // Getter and Setter for isReady (1 bit)
        public boolean isReady() {
            return (flags & IS_READY_MASK) != 0;
        }

        public void setReady(boolean ready) {
            if (ready) {
                flags |= IS_READY_MASK;
            } else {
                flags &= ~IS_READY_MASK;
            }
        }

        // Getter and Setter for errorState (2 bits)
        public int getErrorState() {
            return (flags & ERROR_STATE_MASK) >> ERROR_STATE_SHIFT;
        }

        public void setErrorState(int state) {
            if (state < 0 || state > 3) {
                throw new IllegalArgumentException("Error state must be between 0 and 3.");
            }
            flags = (flags & ~ERROR_STATE_MASK) | (state << ERROR_STATE_SHIFT);
        }

        // Getter and Setter for mode (3 bits)
        public int getMode() {
            return (flags & MODE_MASK) >> MODE_SHIFT;
        }

        public void setMode(int mode) {
            if (mode < 0 || mode > 7) {
                throw new IllegalArgumentException("Mode must be between 0 and 7.");
            }
            flags = (flags & ~MODE_MASK) | (mode << MODE_SHIFT);
        }

        @Override
        public String toString() {
            return String.format(
                "HardwareFlags{isActive=%b, isReady=%b, errorState=%d, mode=%d, rawValue=%d}",
                isActive(), isReady(), getErrorState(), getMode(), flags
            );
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Java Bit Field Simulation Test Cases ---");

        // Test Case 1: All fields off
        HardwareFlags flags1 = new HardwareFlags();
        System.out.println("Test Case 1: " + flags1);

        // Test Case 2: Setting individual fields
        HardwareFlags flags2 = new HardwareFlags();
        flags2.setActive(true);
        flags2.setMode(5);
        System.out.println("Test Case 2: " + flags2);

        // Test Case 3: Setting all fields
        HardwareFlags flags3 = new HardwareFlags();
        flags3.setActive(true);
        flags3.setReady(true);
        flags3.setErrorState(3);
        flags3.setMode(7);
        System.out.println("Test Case 3: " + flags3);
        
        // Test Case 4: Setting and then changing fields
        HardwareFlags flags4 = new HardwareFlags();
        flags4.setReady(true);
        flags4.setErrorState(1);
        flags4.setMode(4);
        System.out.println("Test Case 4 (before): " + flags4);
        flags4.setReady(false);
        flags4.setErrorState(2);
        System.out.println("Test Case 4 (after):  " + flags4);
        
        // Test Case 5: Zero values for multi-bit fields
        HardwareFlags flags5 = new HardwareFlags();
        flags5.setActive(true);
        flags5.setErrorState(0);
        flags5.setMode(0);
        System.out.println("Test Case 5: " + flags5);
    }
}