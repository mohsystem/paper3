public class Task159 {

    /**
     * Securely reads a byte from the buffer at the given index.
     * This method performs bounds checking to prevent IndexOutOfBoundsException.
     *
     * @param buffer The byte array to read from.
     * @param index  The index of the byte to read.
     * @param outValue A single-element array to store the output value. Using an array
     *                 to simulate an 'out' parameter, allowing the method to signal
     *                 success/failure via its return value.
     * @return true if the read was successful, false otherwise (e.g., out of bounds).
     */
    public static boolean secureRead(byte[] buffer, int index, byte[] outValue) {
        if (buffer == null || outValue == null || outValue.length < 1) {
            // Invalid arguments
            return false;
        }

        // Secure bounds check
        if (index >= 0 && index < buffer.length) {
            outValue[0] = buffer[index];
            return true;
        }

        // Index is out of bounds
        return false;
    }

    public static void main(String[] args) {
        int bufferSize = 10;
        // 1. Dynamically allocate a memory buffer
        byte[] buffer = new byte[bufferSize];

        // Initialize buffer with some data
        for (int i = 0; i < bufferSize; i++) {
            buffer[i] = (byte) (i * 10);
        }

        System.out.println("Buffer created with size: " + bufferSize);
        System.out.println("Buffer content: [0, 10, 20, 30, 40, 50, 60, 70, 80, 90]");
        System.out.println("--- Running Java Test Cases ---");

        // 2. Test cases
        int[] testIndices = {0, 5, 9, -1, 10};

        for (int index : testIndices) {
            byte[] result = new byte[1]; // Holder for the output value
            if (secureRead(buffer, index, result)) {
                System.out.println("Reading at index " + index + ": Success! Value = " + result[0]);
            } else {
                System.out.println("Reading at index " + index + ": Error! Index is out of bounds.");
            }
        }
    }
}