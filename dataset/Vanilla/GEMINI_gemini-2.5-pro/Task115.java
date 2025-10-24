public class Task115 {

    /**
     * Simulates C-style bit fields for a date using a single short (16 bits).
     * The bit layout within the 16-bit short is: YYYYYYYMMMMDDDDD
     * - Year: 7 bits (0-127), representing an offset from the year 2000.
     * - Month: 4 bits (1-12)
     * - Day: 5 bits (1-31)
     */
    static class DateBitField {
        private final short data;

        // Define masks to isolate each field
        private static final int DAY_MASK = 0x1F;      // Binary: 0000000000011111
        private static final int MONTH_MASK = 0x1E0;     // Binary: 0000000111100000
        private static final int YEAR_MASK = 0xFE00;    // Binary: 1111111000000000
        
        // Define bit shift counts for each field
        private static final int MONTH_SHIFT = 5;
        private static final int YEAR_SHIFT = 9;

        /**
         * Constructor to create and pack a date into a single short value.
         * @param day Day of the month (1-31)
         * @param month Month of the year (1-12)
         * @param year Year offset from 2000 (0-127)
         */
        public DateBitField(int day, int month, int year) {
            if (day < 1 || day > 31) {
                throw new IllegalArgumentException("Day must be between 1 and 31.");
            }
            if (month < 1 || month > 12) {
                throw new IllegalArgumentException("Month must be between 1 and 12.");
            }
            if (year < 0 || year > 127) {
                throw new IllegalArgumentException("Year offset must be between 0 and 127.");
            }
            
            this.data = (short) ((year << YEAR_SHIFT) | (month << MONTH_SHIFT) | day);
        }

        public int getDay() {
            return this.data & DAY_MASK;
        }

        public int getMonth() {
            return (this.data & MONTH_MASK) >> MONTH_SHIFT;
        }

        public int getYear() {
            // Use unsigned right shift (>>>) to avoid sign extension issues
            return (this.data & YEAR_MASK) >>> YEAR_SHIFT;
        }

        @Override
        public String toString() {
            return String.format("Date: %02d/%02d/%d", getMonth(), getDay(), getYear() + 2000);
        }
    }

    public static void main(String[] args) {
        System.out.println("Java Test Cases (Simulated Bit Fields):");
        
        // Test Case 1
        DateBitField date1 = new DateBitField(1, 1, 23);
        System.out.println(date1);

        // Test Case 2
        DateBitField date2 = new DateBitField(31, 12, 99);
        System.out.println(date2);

        // Test Case 3
        DateBitField date3 = new DateBitField(29, 2, 24);
        System.out.println(date3);

        // Test Case 4
        DateBitField date4 = new DateBitField(15, 8, 47);
        System.out.println(date4);

        // Test Case 5
        DateBitField date5 = new DateBitField(4, 7, 76);
        System.out.println(date5);
    }
}