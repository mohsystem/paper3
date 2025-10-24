#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cctype>
#include <sstream>

using Table = std::vector<std::vector<std::string>>;

Table parse_csv(const std::string& text, char delimiter = ',') {
    Table rows;
    std::vector<std::string> row;
    std::string field;
    bool in_quotes = false;

    for (size_t i = 0; i < text.size(); ++i) {
        char c = text[i];
        if (in_quotes) {
            if (c == '"') {
                if (i + 1 < text.size() && text[i + 1] == '"') {
                    field.push_back('"');
                    ++i;
                } else {
                    in_quotes = false;
                }
            } else {
                field.push_back(c);
            }
        } else {
            if (c == '"') {
                in_quotes = true;
            } else if (c == delimiter) {
                row.emplace_back(field);
                field.clear();
            } else if (c == '\n' || c == '\r') {
                if (c == '\r' && i + 1 < text.size() && text[i + 1] == '\n') {
                    ++i;
                }
                row.emplace_back(field);
                field.clear();
                rows.emplace_back(std::move(row));
                row.clear();
            } else {
                field.push_back(c);
            }
        }
    }
    row.emplace_back(field);
    if (!(rows.empty() && row.size() == 1 && row[0].empty())) {
        rows.emplace_back(std::move(row));
    }
    return rows;
}

std::string to_csv(const Table& rows, char delimiter = ',') {
    std::ostringstream out;
    for (size_t r = 0; r < rows.size(); ++r) {
        const auto& row = rows[r];
        for (size_t c = 0; c < row.size(); ++c) {
            std::string field = row[c];
            bool must_quote = field.find(delimiter) != std::string::npos ||
                              field.find('"') != std::string::npos ||
                              field.find('\n') != std::string::npos ||
                              field.find('\r') != std::string::npos;
            if (must_quote) {
                out << '"';
                for (char ch : field) {
                    if (ch == '"') out << "\"\"";
                    else out << ch;
                }
                out << '"';
            } else {
                out << field;
            }
            if (c + 1 < row.size()) out << delimiter;
        }
        if (r + 1 < rows.size()) out << '\n';
    }
    return out.str();
}

Table select_columns(const Table& rows, const std::vector<size_t>& indices) {
    for (size_t idx : indices) {
        // size_t cannot be negative; we just rely on bounds checks later.
        (void)idx;
    }
    Table out;
    out.reserve(rows.size());
    for (const auto& row : rows) {
        std::vector<std::string> new_row;
        new_row.reserve(indices.size());
        for (size_t idx : indices) {
            if (idx < row.size()) new_row.push_back(row[idx]);
            else new_row.emplace_back("");
        }
        out.emplace_back(std::move(new_row));
    }
    return out;
}

double sum_column(const Table& rows, size_t col_index) {
    double sum = 0.0;
    for (const auto& row : rows) {
        if (col_index < row.size()) {
            const std::string& s = row[col_index];
            // trim
            size_t start = 0, end = s.size();
            while (start < end && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
            while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
            if (start < end) {
                try {
                    sum += std::stod(s.substr(start, end - start));
                } catch (...) {
                    // ignore non-numeric
                }
            }
        }
    }
    return sum;
}

static void print_rows(const Table& rows) {
    std::cout << "Rows: " << rows.size() << "\n";
    for (const auto& row : rows) {
        std::cout << "[";
        for (size_t i = 0; i < row.size(); ++i) {
            std::cout << row[i];
            if (i + 1 < row.size()) std::cout << ", ";
        }
        std::cout << "]\n";
    }
}

int main() {
    // Test 1: Simple CSV
    std::string csv1 = "a,b\nc,d";
    auto rows1 = parse_csv(csv1, ',');
    std::cout << "Test1 parsed:\n";
    print_rows(rows1);
    std::cout << "Test1 serialized:\n" << to_csv(rows1, ',') << "\n\n";

    // Test 2: Quoted with comma
    std::string csv2 = "name,age\n\"Doe, John\",42\n\"Smith, Jane\",35";
    auto rows2 = parse_csv(csv2, ',');
    std::cout << "Test2 parsed:\n";
    print_rows(rows2);
    Table data2(rows2.begin() + 1, rows2.end());
    std::cout << "Sum age: " << sum_column(data2, 1) << "\n\n";

    // Test 3: Newlines in quoted fields and escaped quotes
    std::string csv3 = "id,notes\n1,\"Line1\nLine2\"\n2,\"He said \"\"Hello\"\"\"";
    auto rows3 = parse_csv(csv3, ',');
    std::cout << "Test3 parsed:\n";
    print_rows(rows3);
    std::cout << "Test3 serialized:\n" << to_csv(rows3, ',') << "\n\n";

    // Test 4: Empty fields and trailing delimiters
    std::string csv4 = "a,,c\n,d,\n,,";
    auto rows4 = parse_csv(csv4, ',');
    std::cout << "Test4 parsed:\n";
    print_rows(rows4);
    auto sel4 = select_columns(rows4, {0, 2});
    std::cout << "Test4 select col 0 and 2:\n";
    print_rows(sel4);
    std::cout << "\n";

    // Test 5: Custom delimiter and CRLF
    std::string csv5 = "x;y\r\n1;2\r\n\"3;4\";5";
    auto rows5 = parse_csv(csv5, ';');
    std::cout << "Test5 parsed:\n";
    print_rows(rows5);
    std::cout << "Test5 serialized with ';':\n" << to_csv(rows5, ';') << "\n";

    return 0;
}