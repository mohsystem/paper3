public class Task115 {

    // In Java, bit fields are not a direct language feature.
    // They are simulated using bitwise operations on a primitive integer type.
    //
    // Let's define the structure for an 8-bit status register:
    // Bit 0:      active (1 bit)
    // Bits 1-3:   mode (3 bits, values 0-7)
    // Bits 4-7:   error_code (4 bits, values 0-15)

    // Constants for bit masks and shifts to make the code readable and maintainable.
    private static final int ACTIVE_MASK = 0x01; // 0000 0001
    private static final int MODE_MASK   = 0x0E; // 0000 1110
    private static final int ERROR_CODE_MASK = 0xF0; // 1111 0000

    private static final int ACTIVE_SHIFT = 0;
    private static final int MODE_SHIFT = 1;
    private static final int ERROR_CODE_SHIFT = 4;

    /**
     * Packs individual field values into a single integer.
     *
     * @param active boolean flag (1 bit).
     * @param mode integer value from 0-7 (3 bits).
     * @param errorCode integer value from 0-15 (4 bits).
     * @return An integer representing the packed bit fields.
     */
    public static int packStatus(boolean active, int mode, int errorCode) {
        // Basic validation to ensure values fit in their allocated bits
        if (mode < 0 || mode > 7) {
            throw new IllegalArgumentException("Mode must be between 0 and 7.");
        }
        if (errorCode < 0 || errorCode > 15) {
            throw new IllegalArgumentException("Error code must be between 0 and 15.");
        }

        int activeBit = active ? 1 : 0;
        
        // Use bitwise OR and left shifts to pack the data
        int statusRegister = (activeBit << ACTIVE_SHIFT) | (mode << MODE_SHIFT) | (errorCode << ERROR_CODE_SHIFT);
        return statusRegister;
    }

    /**
     * Unpacks and prints the fields from a given status register integer.
     *
     * @param statusRegister The integer containing the packed bit fields.
     */
    public static void printUnpackedStatus(int statusRegister) {
        // Use bitwise AND and right shifts to extract the data
        boolean active = ((statusRegister & ACTIVE_MASK) >> ACTIVE_SHIFT) == 1;
        int mode = (statusRegister & MODE_MASK) >> MODE_SHIFT;
        int errorCode = (statusRegister & ERROR_CODE_MASK) >> ERROR_CODE_SHIFT;

        System.out.println("  -> Unpacked: active=" + active + ", mode=" + mode + ", errorCode=" + errorCode);
    }

    public static void main(String[] args) {
        System.out.println("Java Bit Field Simulation:");

        // --- Test Cases ---
        int[] testResults = new int[5];
        testResults[0] = packStatus(true, 5, 10);  // Case 1: General case
        testResults[1] = packStatus(false, 0, 0);   // Case 2: All zero values
        testResults[2] = packStatus(true, 7, 15);  // Case 3: All max values
        testResults[3] = packStatus(false, 1, 8);   // Case 4: Another general case
        testResults[4] = packStatus(true, 3, 4);    // Case 5: Mid-range values

        for (int i = 0; i < testResults.length; i++) {
            int status = testResults[i];
            System.out.println("\n--- Test Case " + (i + 1) + " ---");
            System.out.println("Packed Value: " + status + " (Binary: " + String.format("%8s", Integer.toBinaryString(status)).replace(' ', '0') + ")");
            printUnpackedStatus(status);
        }
    }
}