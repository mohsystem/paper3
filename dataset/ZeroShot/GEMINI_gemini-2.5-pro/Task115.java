class Task115 {

    // Internal state representing the bit fields, stored in a 16-bit short.
    private short registerState;

    // Masks and shifts for each field.
    // enable: bit 0 (1 bit)
    private static final int ENABLE_MASK = 0x1;
    private static final int ENABLE_SHIFT = 0;

    // ready: bit 1 (1 bit)
    private static final int READY_MASK = 0x2;
    private static final int READY_SHIFT = 1;

    // mode: bits 2-3 (2 bits)
    private static final int MODE_MASK = 0xC;
    private static final int MODE_SHIFT = 2;
    private static final int MODE_MAX_VALUE = 3;

    // error_code: bits 4-7 (4 bits)
    private static final int ERROR_CODE_MASK = 0xF0;
    private static final int ERROR_CODE_SHIFT = 4;
    private static final int ERROR_CODE_MAX_VALUE = 15;
    
    // reserved: bits 8-15 (8 bits)
    private static final int RESERVED_MASK = 0xFF00;
    private static final int RESERVED_SHIFT = 8;
    private static final int RESERVED_MAX_VALUE = 255;


    public Task115() {
        this.registerState = 0;
    }

    // --- Getter and Setter for 'enable' field ---
    public boolean isEnable() {
        return ((registerState & ENABLE_MASK) >> ENABLE_SHIFT) == 1;
    }

    public void setEnable(boolean enabled) {
        if (enabled) {
            registerState |= ENABLE_MASK;
        } else {
            registerState &= ~ENABLE_MASK;
        }
    }

    // --- Getter and Setter for 'ready' field ---
    public boolean isReady() {
        return ((registerState & READY_MASK) >> READY_SHIFT) == 1;
    }

    public void setReady(boolean ready) {
        if (ready) {
            registerState |= READY_MASK;
        } else {
            registerState &= ~READY_MASK;
        }
    }

    // --- Getter and Setter for 'mode' field ---
    public int getMode() {
        return (registerState & MODE_MASK) >> MODE_SHIFT;
    }

    public void setMode(int mode) {
        if (mode < 0 || mode > MODE_MAX_VALUE) {
            throw new IllegalArgumentException("Mode must be between 0 and " + MODE_MAX_VALUE);
        }
        registerState &= ~MODE_MASK; // Clear current mode bits
        registerState |= (mode << MODE_SHIFT); // Set new mode bits
    }

    // --- Getter and Setter for 'error_code' field ---
    public int getErrorCode() {
        return (registerState & ERROR_CODE_MASK) >> ERROR_CODE_SHIFT;
    }

    public void setErrorCode(int code) {
        if (code < 0 || code > ERROR_CODE_MAX_VALUE) {
            throw new IllegalArgumentException("Error code must be between 0 and " + ERROR_CODE_MAX_VALUE);
        }
        registerState &= ~ERROR_CODE_MASK; // Clear current error code bits
        registerState |= (code << ERROR_CODE_SHIFT); // Set new error code bits
    }
    
    // --- Getter and Setter for 'reserved' field ---
     public int getReserved() {
        return (registerState & RESERVED_MASK) >> RESERVED_SHIFT;
    }
    
     public void setReserved(int value) {
        if (value < 0 || value > RESERVED_MAX_VALUE) {
            throw new IllegalArgumentException("Reserved value must be between 0 and " + RESERVED_MAX_VALUE);
        }
        registerState &= ~RESERVED_MASK;
        registerState |= (value << RESERVED_SHIFT);
    }


    @Override
    public String toString() {
        return String.format(
            "Register(binary=%16s) -> enable=%s, ready=%s, mode=%d, errorCode=%d, reserved=%d",
            Integer.toBinaryString(registerState & 0xFFFF).replace(' ', '0'),
            isEnable(), isReady(), getMode(), getErrorCode(), getReserved()
        );
    }

    public static void main(String[] args) {
        System.out.println("--- Java Bit Field Simulation ---");

        // Test Case 1: Initial state
        System.out.println("\n--- Test Case 1: Initial State ---");
        Task115 reg1 = new Task115();
        System.out.println(reg1);

        // Test Case 2: Setting individual fields
        System.out.println("\n--- Test Case 2: Setting Individual Fields ---");
        Task115 reg2 = new Task115();
        reg2.setEnable(true);
        reg2.setMode(3); // 11 in binary
        reg2.setErrorCode(10); // 1010 in binary
        System.out.println(reg2);

        // Test Case 3: Setting all fields
        System.out.println("\n--- Test Case 3: Setting All Fields ---");
        Task115 reg3 = new Task115();
        reg3.setEnable(true);
        reg3.setReady(true);
        reg3.setMode(1);
        reg3.setErrorCode(5);
        reg3.setReserved(255);
        System.out.println(reg3);

        // Test Case 4: Modifying existing fields
        System.out.println("\n--- Test Case 4: Modifying Existing Fields ---");
        reg3.setEnable(false);
        reg3.setErrorCode(15);
        System.out.println(reg3);
        
        // Test Case 5: Invalid input
        System.out.println("\n--- Test Case 5: Invalid Input ---");
        Task115 reg5 = new Task115();
        try {
            reg5.setMode(4);
        } catch (IllegalArgumentException e) {
            System.out.println("Caught expected exception: " + e.getMessage());
        }
        try {
            reg5.setErrorCode(-1);
        } catch (IllegalArgumentException e) {
            System.out.println("Caught expected exception: " + e.getMessage());
        }
        System.out.println("State after invalid attempts: " + reg5);
    }
}