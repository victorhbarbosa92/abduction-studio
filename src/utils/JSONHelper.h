#pragma once
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <iostream>

namespace KuroUI {
    // A simple, dependency-free JSON generator/parser
    class JSON {
    public:
        // Helper to stringify simple types
        static std::string escape(const std::string& str) {
            return "\"" + str + "\"";
        }
        
        static std::string format(const std::string& key, float val) {
            return escape(key) + ": " + std::to_string(val);
        }
        
        static std::string format(const std::string& key, int val) {
            return escape(key) + ": " + std::to_string(val);
        }
        
        static std::string format(const std::string& key, bool val) {
            return escape(key) + ": " + (val ? "true" : "false");
        }
        
        static std::string format(const std::string& key, const std::string& val) {
            return escape(key) + ": " + escape(val);
        }

        // Helper to parse simple values using string matching (robust enough for our format)
        static std::string parseString(const std::string& json, const std::string& key) {
            size_t pos = json.find(escape(key));
            if (pos == std::string::npos) return "";
            pos = json.find(":", pos);
            if (pos == std::string::npos) return "";
            size_t start = json.find("\"", pos);
            if (start == std::string::npos) return "";
            size_t end = json.find("\"", start + 1);
            if (end == std::string::npos) return "";
            return json.substr(start + 1, end - start - 1);
        }
        
        static float parseFloat(const std::string& json, const std::string& key, float default_val = 0.0f) {
            size_t pos = json.find(escape(key));
            if (pos == std::string::npos) return default_val;
            pos = json.find(":", pos);
            if (pos == std::string::npos) return default_val;
            try {
                return std::stof(json.substr(pos + 1));
            } catch (...) {
                return default_val;
            }
        }
        
        static int parseInt(const std::string& json, const std::string& key, int default_val = 0) {
            size_t pos = json.find(escape(key));
            if (pos == std::string::npos) return default_val;
            pos = json.find(":", pos);
            if (pos == std::string::npos) return default_val;
            try {
                return std::stoi(json.substr(pos + 1));
            } catch (...) {
                return default_val;
            }
        }
        
        static bool parseBool(const std::string& json, const std::string& key, bool default_val = false) {
            size_t pos = json.find(escape(key));
            if (pos == std::string::npos) return default_val;
            pos = json.find(":", pos);
            if (pos == std::string::npos) return default_val;
            size_t true_pos = json.find("true", pos);
            size_t false_pos = json.find("false", pos);
            if (true_pos != std::string::npos && (false_pos == std::string::npos || true_pos < false_pos)) {
                return true;
            }
            return false;
        }

        // Parse list/arrays of floats
        static std::vector<float> parseFloatArray(const std::string& json, const std::string& key) {
            std::vector<float> res;
            size_t pos = json.find(escape(key));
            if (pos == std::string::npos) return res;
            pos = json.find("[", pos);
            if (pos == std::string::npos) return res;
            size_t end = json.find("]", pos);
            if (end == std::string::npos) return res;
            
            std::string arr_str = json.substr(pos + 1, end - pos - 1);
            std::replace(arr_str.begin(), arr_str.end(), ',', ' ');
            std::stringstream ss(arr_str);
            float val;
            while (ss >> val) {
                res.push_back(val);
            }
            return res;
        }
        
        // Parse list/arrays of ints
        static std::vector<int> parseIntArray(const std::string& json, const std::string& key) {
            std::vector<int> res;
            size_t pos = json.find(escape(key));
            if (pos == std::string::npos) return res;
            pos = json.find("[", pos);
            if (pos == std::string::npos) return res;
            size_t end = json.find("]", pos);
            if (end == std::string::npos) return res;
            
            std::string arr_str = json.substr(pos + 1, end - pos - 1);
            std::replace(arr_str.begin(), arr_str.end(), ',', ' ');
            std::stringstream ss(arr_str);
            int val;
            while (ss >> val) {
                res.push_back(val);
            }
            return res;
        }

        // Returns substring of an object block: e.g. "tracks": [ ... ] or "synths": { ... }
        static std::string getSubObject(const std::string& json, const std::string& key) {
            size_t pos = json.find(escape(key));
            if (pos == std::string::npos) return "";
            pos = json.find(":", pos);
            if (pos == std::string::npos) return "";
            // Find starting bracket/brace
            size_t start = json.find_first_of("{[", pos);
            if (start == std::string::npos) return "";
            
            char open_char = json[start];
            char close_char = (open_char == '{') ? '}' : ']';
            
            int count = 1;
            size_t i = start + 1;
            for (; i < json.size(); ++i) {
                if (json[i] == open_char) count++;
                else if (json[i] == close_char) {
                    count--;
                    if (count == 0) break;
                }
            }
            if (count != 0) return "";
            return json.substr(start, i - start + 1);
        }

        // Splits array of objects: [ {...}, {...} ] into individual {...} strings
        static std::vector<std::string> splitArrayObjects(const std::string& array_json) {
            std::vector<std::string> res;
            size_t start = array_json.find("[");
            size_t end = array_json.rfind("]");
            if (start == std::string::npos || end == std::string::npos || start >= end) return res;
            
            std::string inner = array_json.substr(start + 1, end - start - 1);
            int brace_count = 0;
            size_t obj_start = std::string::npos;
            
            for (size_t i = 0; i < inner.size(); ++i) {
                if (inner[i] == '{') {
                    if (brace_count == 0) obj_start = i;
                    brace_count++;
                } else if (inner[i] == '}') {
                    brace_count--;
                    if (brace_count == 0 && obj_start != std::string::npos) {
                        res.push_back(inner.substr(obj_start, i - obj_start + 1));
                        obj_start = std::string::npos;
                    }
                }
            }
            return res;
        }
    };
}
