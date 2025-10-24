#include <iostream>
#include <vector>
#include <string>

class Task119 {
public:
    static std::vector<std::vector<std::string>> parseCSV(const std::string& csv) {
        std::vector<std::vector<std::string>> rows;
        if (csv.empty()) return rows;

        std::vector<std::string> currentRow;
        std::string field;
        bool inQuotes = false;
        bool fieldStart = true;

        const size_t n = csv.size();
        for (size_t i = 0; i < n; ++i) {
            char ch = csv[i];
            if (inQuotes) {
                if (ch == '"') {
                    if (i + 1 < n && csv[i + 1] == '"') {
                        field.push_back('"');
                        ++i;
                    } else {
                        inQuotes = false;
                    }
                } else {
                    field.push_back(ch);
                }
            } else {
                if (fieldStart && ch == '"') {
                    inQuotes = true;
                    fieldStart = false;
                } else if (ch == ',') {
                    currentRow.emplace_back(field);
                    field.clear();
                    fieldStart = true;
                } else if (ch == '\n') {
                    currentRow.emplace_back(field);
                    field.clear();
                    rows.emplace_back(std::move(currentRow));
                    currentRow = std::vector<std::string>();
                    fieldStart = true;
                } else if (ch == '\r') {
                    if (i + 1 < n && csv[i + 1] == '\n') {
                        ++i;
                    }
                    currentRow.emplace_back(field);
                    field.clear();
                    rows.emplace_back(std::move(currentRow));
                    currentRow = std::vector<std::string>();
                    fieldStart = true;
                } else {
                    field.push_back(ch);
                    fieldStart = false;
                }
            }
        }
        if (!field.empty() || !fieldStart || !currentRow.empty() || (!csv.empty() && csv.back() == ',')) {
            currentRow.emplace_back(field);
        }
        if (!currentRow.empty()) {
            rows.emplace_back(std::move(currentRow));
        }
        return rows;
    }
};

static std::string formatRows(const std::vector<std::vector<std::string>>& rows) {
    std::string out;
    out += "Rows=" + std::to_string(rows.size()) + "\n";
    for (size_t r = 0; r < rows.size(); ++r) {
        out += "Row " + std::to_string(r) + ": [";
        for (size_t c = 0; c < rows[r].size(); ++c) {
            std::string safe;
            for (char ch : rows[r][c]) {
                if (ch == '\\' || ch == '"') safe.push_back('\\');
                safe.push_back(ch);
            }
            out += "\"" + safe + "\"";
            if (c + 1 < rows[r].size()) out += ", ";
        }
        out += "]\n";
    }
    return out;
}

int main() {
    std::vector<std::string> tests = {
        "name,age,city\nAlice,30,Seattle",
        "a,\"b,b\",c",
        "row1col1,row1col2\r\nrow2col1,\"row2\ncol2\",row2col3",
        "\"He said \"\"Hello\"\"\",42",
        "1,2,3,\n, ,\"\""
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        auto parsed = Task119::parseCSV(tests[i]);
        std::cout << "Test " << (i + 1) << ":\n";
        std::cout << formatRows(parsed);
    }
    return 0;
}