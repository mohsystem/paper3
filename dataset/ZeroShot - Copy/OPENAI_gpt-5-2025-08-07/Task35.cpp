#include <bits/stdc++.h>
using namespace std;

long long countPassengers(const vector<pair<long long, long long>>& stops) {
    long long count = 0;
    for (size_t i = 0; i < stops.size(); ++i) {
        long long on = stops[i].first;
        long long off = stops[i].second;
        if (on < 0 || off < 0) {
            throw invalid_argument("Values must be non-negative");
        }
        if (i == 0 && off != 0) {
            throw invalid_argument("At first stop, number of people getting off must be zero");
        }
        if (on > LLONG_MAX - count) {
            throw overflow_error("Overflow while adding passengers");
        }
        long long afterBoard = count + on;
        if (off > afterBoard) {
            throw invalid_argument("More people getting off than currently on the bus");
        }
        count = afterBoard - off;
    }
    return count;
}

static void runTest(const vector<pair<long long,long long>>& stops) {
    try {
        long long res = countPassengers(stops);
        cout << "Stops: [";
        for (size_t i = 0; i < stops.size(); ++i) {
            cout << "(" << stops[i].first << "," << stops[i].second << ")";
            if (i + 1 != stops.size()) cout << ", ";
        }
        cout << "] -> Remaining: " << res << "\n";
    } catch (const exception& e) {
        cout << "Error: " << e.what() << "\n";
    }
}

int main() {
    vector<pair<long long,long long>> t1{{10,0},{3,5},{5,8}};
    vector<pair<long long,long long>> t2{{3,0},{9,1},{4,10},{12,2},{6,1},{7,10}};
    vector<pair<long long,long long>> t3{{0,0}};
    vector<pair<long long,long long>> t4{{5,0},{0,0},{0,0}};
    vector<pair<long long,long long>> t5{{1000000000LL,0},{1000000000LL,0},{0,1000000000LL}};

    runTest(t1);
    runTest(t2);
    runTest(t3);
    runTest(t4);
    runTest(t5);

    return 0;
}