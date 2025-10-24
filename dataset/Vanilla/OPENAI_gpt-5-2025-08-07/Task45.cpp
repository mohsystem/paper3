#include <bits/stdc++.h>
using namespace std;

class Task45 {
public:
    static string processInput(const string& input) {
        string s = input;
        // Trim
        auto ltrim = [](string& str) {
            str.erase(str.begin(), find_if(str.begin(), str.end(), [](unsigned char ch){ return !isspace(ch); }));
        };
        auto rtrim = [](string& str) {
            str.erase(find_if(str.rbegin(), str.rend(), [](unsigned char ch){ return !isspace(ch); }).base(), str.end());
        };
        ltrim(s); rtrim(s);
        if (s.empty()) return "ERROR: Empty input";

        try {
            istringstream iss(s);
            string op;
            if (!(iss >> op)) return "ERROR: Empty input";
            for (auto& c : op) c = (char)tolower(c);

            vector<double> nums;
            string tok;
            while (iss >> tok) {
                try {
                    size_t idx = 0;
                    double v = stod(tok, &idx);
                    if (idx != tok.size()) return string("ERROR: Invalid number '") + tok + "'";
                    nums.push_back(v);
                } catch (...) {
                    return string("ERROR: Invalid number '") + tok + "'";
                }
            }

            auto finite = [](double x){ return isfinite(x); };
            auto fmt = [](double v){
                // If near integer, print integer
                double r = nearbyint(v);
                if (fabs(v - r) < 1e-12 && fabs(r) < 9.22e18) {
                    ostringstream oss;
                    oss << (long long) llround(r);
                    return oss.str();
                } else {
                    ostringstream oss;
                    oss.setf(std::ios::fmtflags(0), std::ios::floatfield);
                    oss << setprecision(12) << v;
                    string out = oss.str();
                    // Remove trailing zeros in decimal part
                    if (out.find('.') != string::npos) {
                        // Remove trailing zeros
                        while (!out.empty() && out.back() == '0') out.pop_back();
                        if (!out.empty() && out.back() == '.') out.pop_back();
                    }
                    return out;
                }
            };

            if (op == "add") {
                if (nums.size() < 2) return "ERROR: 'add' requires at least 2 numbers";
                double res = 0.0;
                for (double v : nums) res += v;
                if (!finite(res)) return "ERROR: Numeric overflow";
                return fmt(res);
            } else if (op == "mul") {
                if (nums.size() < 2) return "ERROR: 'mul' requires at least 2 numbers";
                double res = 1.0;
                for (double v : nums) res *= v;
                if (!finite(res)) return "ERROR: Numeric overflow";
                return fmt(res);
            } else if (op == "sub") {
                if (nums.size() != 2) return "ERROR: 'sub' requires exactly 2 numbers";
                double res = nums[0] - nums[1];
                if (!finite(res)) return "ERROR: Numeric overflow";
                return fmt(res);
            } else if (op == "div") {
                if (nums.size() != 2) return "ERROR: 'div' requires exactly 2 numbers";
                if (nums[1] == 0.0) return "ERROR: Division by zero";
                double res = nums[0] / nums[1];
                if (!finite(res)) return "ERROR: Numeric overflow";
                return fmt(res);
            } else if (op == "pow") {
                if (nums.size() != 2) return "ERROR: 'pow' requires exactly 2 numbers";
                double res = pow(nums[0], nums[1]);
                if (!finite(res)) return "ERROR: Numeric overflow";
                return fmt(res);
            } else {
                return string("ERROR: Unknown operation '") + op + "'";
            }
        } catch (...) {
            return "ERROR: Unexpected error";
        }
    }
};

int main() {
    vector<string> tests = {
        "add 2 3 4",
        "div 10 0",
        "pow 2 10",
        "mul 1.5 4",
        "sub 5 abc"
    };
    for (auto& t : tests) {
        cout << t << " => " << Task45::processInput(t) << "\n";
    }
    return 0;
}