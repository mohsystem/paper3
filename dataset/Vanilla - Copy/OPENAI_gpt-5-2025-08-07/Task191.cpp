#include <bits/stdc++.h>
using namespace std;

class Task191 {
public:
    static int longestValidParentheses(const string& s) {
        vector<int> st;
        st.push_back(-1);
        int maxLen = 0;
        for (int i = 0; i < (int)s.size(); ++i) {
            if (s[i] == '(') {
                st.push_back(i);
            } else {
                if (!st.empty()) st.pop_back();
                if (st.empty()) {
                    st.push_back(i);
                } else {
                    maxLen = max(maxLen, i - st.back());
                }
            }
        }
        return maxLen;
    }
};

int main() {
    vector<string> tests = {
        "(()",
        ")()())",
        "",
        "()(())",
        "())((())"
    };
    for (const auto& t : tests) {
        cout << "Input: \"" << t << "\" => Output: " << Task191::longestValidParentheses(t) << "\n";
    }
    return 0;
}