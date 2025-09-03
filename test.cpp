#include <bits/stdc++.h>
#include "json.hpp"   // Download from https://github.com/nlohmann/json

using namespace std;
using json = nlohmann::json;

// Convert string from given base to decimal
long long baseToDecimal(const string &val, int base) {
    long long result = 0;
    for (char c : val) {
        int digit;
        if (isdigit(c)) digit = c - '0';
        else if (isalpha(c)) digit = 10 + (tolower(c) - 'a');
        else continue;
        result = result * base + digit;
    }
    return result;
}

// Lagrange interpolation at x=0 → constant term
long long lagrangeInterpolation(const vector<pair<long long,long long>> &points, int k) {
    long double result = 0.0;
    for (int i = 0; i < k; i++) {
        long double term = points[i].second;
        for (int j = 0; j < k; j++) {
            if (i != j) {
                term *= (0.0 - points[j].first) / (points[i].first - points[j].first);
            }
        }
        result += term;
    }
    return llround(result); // nearest integer
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Read entire input (may contain multiple JSON blocks)
    string input((istreambuf_iterator<char>(cin)), istreambuf_iterator<char>());

    // Extract JSON testcases one by one
    vector<json> testcases;
    size_t pos = 0;
    while (pos < input.size()) {
        size_t start = input.find('{', pos);
        if (start == string::npos) break;
        int depth = 0;
        size_t end = start;
        do {
            if (input[end] == '{') depth++;
            else if (input[end] == '}') depth--;
            end++;
        } while (end < input.size() && depth > 0);

        string block = input.substr(start, end - start);
        testcases.push_back(json::parse(block));
        pos = end;
    }

    // Process each testcase
    for (auto &data : testcases) {
        int n = data["keys"]["n"];
        int k = data["keys"]["k"];

        vector<pair<long long,long long>> points;

        // Build (x,y) pairs
        for (auto& el : data.items()) {
            if (el.key() == "keys") continue;
            long long x = stoll(el.key());
            string val = el.value()["value"];
            int base = stoi(el.value()["base"]);
            long long y = baseToDecimal(val, base);
            points.push_back({x, y});
        }

        // Sort by x
        sort(points.begin(), points.end());

        // Take first k points (Shamir requires any k, here we just pick first k)
        vector<pair<long long,long long>> subset(points.begin(), points.begin() + k);

        long long c = lagrangeInterpolation(subset, k);

        cout << c << "\n"; // Output constant term
    }

    return 0;
}
