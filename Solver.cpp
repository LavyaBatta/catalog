#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>

// Single-header library for JSON parsing
#include "json.hpp"

// Boost Multiprecision for handling very large integers
// You will need to have the Boost libraries available.
// See instructions below.
#include <boost/multiprecision/cpp_int.hpp>

using json = nlohmann::json;
using BigInt = boost::multiprecision::cpp_int;

// A structure to hold our points, using BigInt for y
struct Point {
    BigInt x;
    BigInt y;
};

// Converts a character ('0'-'9', 'a'-'f', etc.) to its integer value
int charToValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'z') return c - 'a' + 10;
    if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
    throw std::invalid_argument("Invalid character in number value");
}

// Converts a number string from a given base to a BigInt decimal
BigInt convertToDecimal(const std::string& valueStr, int base) {
    BigInt result = 0;
    BigInt power = 1;

    for (int i = valueStr.length() - 1; i >= 0; i--) {
        int digitValue = charToValue(valueStr[i]);
        if (digitValue >= base) {
            throw std::runtime_error("Digit value out of range for the given base.");
        }
        result += digitValue * power;
        power *= base;
    }
    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input.json>" << std::endl;
        return 1;
    }

    std::ifstream inputFile(argv[1]);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;
        return 1;
    }

    json data;
    try {
        data = json::parse(inputFile);
    } catch (json::parse_error& e) {
        std::cerr << "Error: JSON parsing failed: " << e.what() << std::endl;
        return 1;
    }

    try {
        // 1. Get k, the number of points needed
        int k = data.at("keys").at("k").get<int>();
        std::vector<Point> points;

        // 2. Extract the first k points, converting them to decimal
        for (auto& [key, val] : data.items()) {
            if (key == "keys") continue; // Skip the metadata key
            if (points.size() >= k) break; // Stop once we have enough points

            BigInt x = std::stoi(key);
            int base = std::stoi(val.at("base").get<std::string>());
            std::string value_str = val.at("value").get<std::string>();
            BigInt y = convertToDecimal(value_str, base);
            
            points.push_back({x, y});
        }
        
        if (points.size() < k) {
            throw std::runtime_error("Not enough points in JSON to solve the polynomial.");
        }

        // 3. Apply generalized Lagrange Interpolation to find P(0)
        BigInt constant_c = 0;
        for (int j = 0; j < k; ++j) {
            BigInt numerator = 1;
            BigInt denominator = 1;
            
            for (int i = 0; i < k; ++i) {
                if (i == j) continue;
                numerator *= (0 - points[i].x);
                denominator *= (points[j].x - points[i].x);
            }
            
            // The formula is y_j * (numerator / denominator).
            // To avoid floating point division, we rearrange to (y_j * numerator) / denominator
            BigInt term = (points[j].y * numerator) / denominator;
            constant_c += term;
        }

        // 4. Format and print the output as JSON
        json result;
        result["constant"] = constant_c.str(); // Convert BigInt to string for output
        std::cout << result.dump(2) << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}