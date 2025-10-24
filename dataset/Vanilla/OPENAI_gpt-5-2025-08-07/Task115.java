import java.util.Arrays;

public class Task115 {
    // Packet class representing fields as if they were bit fields
    public static class Packet {
        public final int version;  // 3 bits
        public final int type;     // 5 bits
        public final int flags;    // 4 bits
        public final int length;   // 12 bits
        public final int checksum; // 8 bits

        public Packet(int version, int type, int flags, int length, int checksum) {
            this.version = version & 0x7;
            this.type = type & 0x1F;
            this.flags = flags & 0xF;
            this.length = length & 0xFFF;
            this.checksum = checksum & 0xFF;
        }

        @Override
        public String toString() {
            return "Packet{version=" + version + ", type=" + type + ", flags=" + flags +
                    ", length=" + length + ", checksum=" + checksum + "}";
        }
    }

    // Packs the fields into a 32-bit int: [checksum:8][length:12][flags:4][type:5][version:3]
    public static int packPacket(int version, int type, int flags, int length, int checksum) {
        int v = version & 0x7;
        int t = type & 0x1F;
        int f = flags & 0xF;
        int l = length & 0xFFF;
        int c = checksum & 0xFF;
        return (c << 24) | (l << 12) | (f << 8) | (t << 3) | v;
    }

    // Unpacks the 32-bit int into a Packet
    public static Packet unpackPacket(int value) {
        int v = value & 0x7;
        int t = (value >>> 3) & 0x1F;
        int f = (value >>> 8) & 0xF;
        int l = (value >>> 12) & 0xFFF;
        int c = (value >>> 24) & 0xFF;
        return new Packet(v, t, f, l, c);
    }

    private static void runTest(int version, int type, int flags, int length, int checksum) {
        int packed = packPacket(version, type, flags, length, checksum);
        Packet p = unpackPacket(packed);
        System.out.println("Input  : " + Arrays.asList(version, type, flags, length, checksum));
        System.out.printf("Packed : 0x%08X%n", packed);
        System.out.println("Unpack : " + p);
        System.out.println("---");
    }

    public static void main(String[] args) {
        runTest(1, 3, 2, 100, 200);
        runTest(7, 31, 15, 4095, 255);
        runTest(0, 0, 0, 0, 0);
        runTest(5, 10, 8, 2048, 128);
        runTest(2, 18, 7, 1234, 77);
    }
}