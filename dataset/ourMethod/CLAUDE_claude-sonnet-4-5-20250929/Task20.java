
public class Task20 {
    public static long digPow(long n, int p) {
        if (n <= 0 || p <= 0) {
            return -1;
        }
        
        String nStr = Long.toString(n);
        if (nStr.length() > 18) {
            return -1;
        }
        
        long sum = 0;
        int power = p;
        
        for (int i = 0; i < nStr.length(); i++) {
            int digit = nStr.charAt(i) - '0';
            if (digit < 0 || digit > 9) {
                return -1;
            }
            
            if (power > 63) {
                return -1;
            }
            
            long powResult = 1;
            for (int j = 0; j < power; j++) {
                if (powResult > Long.MAX_VALUE / digit) {
                    return -1;
                }
                powResult *= digit;
            }
            
            if (sum > Long.MAX_VALUE - powResult) {
                return -1;
            }
            sum += powResult;
            power++;
        }
        
        if (sum % n == 0) {
            return sum / n;
        }
        
        return -1;
    }
    
    public static void main(String[] args) {
        System.out.println(digPow(89, 1));
        System.out.println(digPow(92, 1));
        System.out.println(digPow(695, 2));
        System.out.println(digPow(46288, 3));
        System.out.println(digPow(1, 1));
    }
}
