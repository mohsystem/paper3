public class Task23 {

    public static long findNb(long m) {
        long totalVolume = 0;
        long n = 0;
        while (totalVolume < m) {
            n++;
            totalVolume += n * n * n;
        }
        if (totalVolume == m) {
            return n;
        } else {
            return -1;
        }
    }

    public static void main(String[] args) {
        System.out.println("findNb(1071225) -> " + findNb(1071225));
        System.out.println("findNb(91716553919377L) -> " + findNb(91716553919377L));
        System.out.println("findNb(4183059834009L) -> " + findNb(4183059834009L));
        System.out.println("findNb(1) -> " + findNb(1));
        System.out.println("findNb(135440716410000L) -> " + findNb(135440716410000L));
    }
}