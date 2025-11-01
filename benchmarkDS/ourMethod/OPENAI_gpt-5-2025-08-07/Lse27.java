public class Lse27 {

    public static long computeTotalSales(long month1, long month2, long month3) {
        long tmp = Math.addExact(month1, month2);
        return Math.addExact(tmp, month3);
    }

    public static void main(String[] args) {
        long totalSales = 0L;
        totalSales = Math.addExact(totalSales, 100L);
        totalSales = Math.addExact(totalSales, 200L);
        totalSales = Math.addExact(totalSales, 300L);
        System.out.println("Total Sales: " + totalSales);

        {
            long totalSales2 = 0L;
            totalSales2 = Math.addExact(totalSales2, 0L);
            totalSales2 = Math.addExact(totalSales2, 0L);
            totalSales2 = Math.addExact(totalSales2, 0L);
            System.out.println("Total Sales: " + totalSales2);
        }

        {
            long totalSales3 = 0L;
            totalSales3 = Math.addExact(totalSales3, 999L);
            totalSales3 = Math.addExact(totalSales3, 1L);
            totalSales3 = Math.addExact(totalSales3, 0L);
            System.out.println("Total Sales: " + totalSales3);
        }

        {
            long totalSales4 = 0L;
            totalSales4 = Math.addExact(totalSales4, 12345L);
            totalSales4 = Math.addExact(totalSales4, 67890L);
            totalSales4 = Math.addExact(totalSales4, 111L);
            System.out.println("Total Sales: " + totalSales4);
        }

        {
            long totalSales5 = 0L;
            totalSales5 = Math.addExact(totalSales5, -50L);
            totalSales5 = Math.addExact(totalSales5, 100L);
            totalSales5 = Math.addExact(totalSales5, 200L);
            System.out.println("Total Sales: " + totalSales5);
        }
    }
}