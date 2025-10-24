
import java.util.Objects;

public class Task115 {
    // Class representing a configuration with bit-field-like properties
    public static class BitFieldConfig {
        private int value;
        
        // Bit masks and shifts for different fields
        private static final int FLAG1_MASK = 0x01;  // 1 bit
        private static final int FLAG2_MASK = 0x02;  // 1 bit
        private static final int MODE_MASK = 0x1C;   // 3 bits
        private static final int MODE_SHIFT = 2;
        private static final int LEVEL_MASK = 0xE0;  // 3 bits
        private static final int LEVEL_SHIFT = 5;
        
        public BitFieldConfig() {
            this.value = 0;
        }
        
        public BitFieldConfig(int initialValue) {
            if (initialValue < 0 || initialValue > 0xFF) {
                throw new IllegalArgumentException("Initial value must be between 0 and 255");
            }
            this.value = initialValue;
        }
        
        public boolean getFlag1() {
            return (value & FLAG1_MASK) != 0;
        }
        
        public void setFlag1(boolean flag) {
            if (flag) {
                value |= FLAG1_MASK;
            } else {
                value &= ~FLAG1_MASK;
            }
        }
        
        public boolean getFlag2() {
            return (value & FLAG2_MASK) != 0;
        }
        
        public void setFlag2(boolean flag) {
            if (flag) {
                value |= FLAG2_MASK;
            } else {
                value &= ~FLAG2_MASK;
            }
        }
        
        public int getMode() {
            return (value & MODE_MASK) >> MODE_SHIFT;
        }
        
        public void setMode(int mode) {
            if (mode < 0 || mode > 7) {
                throw new IllegalArgumentException("Mode must be between 0 and 7");
            }
            value = (value & ~MODE_MASK) | ((mode << MODE_SHIFT) & MODE_MASK);
        }
        
        public int getLevel() {
            return (value & LEVEL_MASK) >> LEVEL_SHIFT;
        }
        
        public void setLevel(int level) {
            if (level < 0 || level > 7) {
                throw new IllegalArgumentException("Level must be between 0 and 7");
            }
            value = (value & ~LEVEL_MASK) | ((level << LEVEL_SHIFT) & LEVEL_MASK);
        }
        
        public int getValue() {
            return value;
        }
        
        @Override
        public String toString() {
            return String.format("BitFieldConfig{flag1=%b, flag2=%b, mode=%d, level=%d, value=0x%02X}",
                    getFlag1(), getFlag2(), getMode(), getLevel(), value);
        }
        
        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (obj == null || getClass() != obj.getClass()) return false;
            BitFieldConfig that = (BitFieldConfig) obj;
            return value == that.value;
        }
        
        @Override
        public int hashCode() {
            return Objects.hash(value);
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Default constructor
        System.out.println("Test 1: Default constructor");
        BitFieldConfig config1 = new BitFieldConfig();
        System.out.println(config1);
        System.out.println();
        
        // Test case 2: Set individual fields
        System.out.println("Test 2: Set individual fields");
        BitFieldConfig config2 = new BitFieldConfig();
        config2.setFlag1(true);
        config2.setFlag2(false);
        config2.setMode(3);
        config2.setLevel(5);
        System.out.println(config2);
        System.out.println();
        
        // Test case 3: Initialize with value
        System.out.println("Test 3: Initialize with value");
        BitFieldConfig config3 = new BitFieldConfig(0xAD);
        System.out.println(config3);
        System.out.println();
        
        // Test case 4: Modify existing configuration
        System.out.println("Test 4: Modify existing configuration");
        BitFieldConfig config4 = new BitFieldConfig(0x55);
        System.out.println("Before: " + config4);
        config4.setFlag1(false);
        config4.setMode(7);
        System.out.println("After:  " + config4);
        System.out.println();
        
        // Test case 5: Boundary values
        System.out.println("Test 5: Boundary values");
        BitFieldConfig config5 = new BitFieldConfig();
        config5.setFlag1(true);
        config5.setFlag2(true);
        config5.setMode(7);
        config5.setLevel(7);
        System.out.println(config5);
        System.out.println("Max value: 0x" + Integer.toHexString(config5.getValue()).toUpperCase());
    }
}
