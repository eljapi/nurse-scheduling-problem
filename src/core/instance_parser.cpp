#include "instance_parser.h"
#include <fstream>
#include <iostream>

// Section identifiers from original code
const std::string sections[7] = {
    "SECTION_HORIZON",
    "SECTION_SHIFTS", 
    "SECTION_STAFF",
    "SECTION_DAYS_OFF",
    "SECTION_SHIFT_ON_REQUESTS",
    "SECTION_SHIFT_OFF_REQUESTS",
    "SECTION_COVER"
};

std::vector<std::string> InstanceParser::tokenize(const std::string& s, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = s.find(delimiter);
    
    while (end != std::string::npos) {
        std::string token = s.substr(start, end - start);
        // Trim whitespace
        while (!token.empty() && (token.back() == ' ' || token.back() == '\r' || token.back() == '\n')) {
            token.pop_back();
        }
        while (!token.empty() && (token.front() == ' ' || token.front() == '\r' || token.front() == '\n')) {
            token.erase(0, 1);
        }
        tokens.push_back(token);
        start = end + delimiter.size();
        end = s.find(delimiter, start);
    }
    
    std::string token = s.substr(start);
    // Trim whitespace from last token
    while (!token.empty() && (token.back() == ' ' || token.back() == '\r' || token.back() == '\n')) {
        token.pop_back();
    }
    while (!token.empty() && (token.front() == ' ' || token.front() == '\r' || token.front() == '\n')) {
        token.erase(0, 1);
    }
    tokens.push_back(token);
    return tokens;
}

std::vector<std::string> InstanceParser::cantFollow(const std::string& s, const std::string& delimiter) {
    if (s.empty()) {
        return std::vector<std::string>();
    }
    return tokenize(s, delimiter);
}

std::string InstanceParser::parseShift(const std::string& s, const std::string& delimiter) {
    // This mimics the original parseShift function
    // For "D=14" with delimiter "=", it should return "14"
    size_t start = 0;
    size_t end = s.find(delimiter);
    
    while (end != std::string::npos) {
        start = end + delimiter.size();
        end = s.find(delimiter, start);
    }
    
    std::string result = s.substr(start);
    return result;
}

std::vector<std::string> InstanceParser::maxshift(const std::string& s, const std::string& delimiter) {
    std::vector<std::string> shiftTypes;
    auto tokens = tokenize(s, delimiter);
    
    for (const auto& token : tokens) {
        std::string shiftType = parseShift(token, "=");
        shiftTypes.push_back(shiftType);
    }
    return shiftTypes;
}

void InstanceParser::addShift(Shift* shift, const std::string& s, const std::string& delimiter) {
    auto tokens = tokenize(s, delimiter);
    
    if (tokens.size() >= 2) {
        shift->ShiftID = tokens[0];
        shift->mins = std::stoi(tokens[1]);
        
        if (tokens.size() >= 3 && !tokens[2].empty()) {
            shift->cant_follow = cantFollow(tokens[2], "|");
        }
    }
}

void InstanceParser::addStaff(Staff* staff, const std::string& s, const std::string& delimiter) {
    auto tokens = tokenize(s, delimiter);
    
    if (tokens.size() >= 8) {
        staff->ID = tokens[0];
        staff->MaxShifts = maxshift(tokens[1], "|");
        staff->MaxTotalMinutes = std::stoi(tokens[2]);
        staff->MinTotalMinutes = std::stoi(tokens[3]);
        staff->MaxConsecutiveShifts = std::stoi(tokens[4]);
        staff->MinConsecutiveShifts = std::stoi(tokens[5]);
        staff->MinConsecutiveDaysOff = std::stoi(tokens[6]);
        staff->MaxWeekends = std::stoi(tokens[7]);
    }
}

void InstanceParser::addDaysOff(DaysOff* section, const std::string& s, const std::string& delimiter) {
    auto tokens = tokenize(s, ",");
    
    if (!tokens.empty()) {
        section->EmployeeID = tokens[0];
        for (size_t i = 1; i < tokens.size(); i++) {
            section->DayIndexes.push_back(tokens[i]);
        }
    }
}

void InstanceParser::addShiftOnRequest(ShiftOnRequest* section, const std::string& s, const std::string& delimiter) {
    auto tokens = tokenize(s, ",");
    
    if (tokens.size() >= 4) {
        section->EmployeeID = tokens[0];
        section->Day = std::stoi(tokens[1]);
        section->ShiftID = tokens[2];
        section->Weight = std::stoi(tokens[3]);
    }
}

void InstanceParser::addShiftOffRequest(ShiftOffRequest* section, const std::string& s, const std::string& delimiter) {
    auto tokens = tokenize(s, ",");
    
    if (tokens.size() >= 4) {
        section->EmployeeID = tokens[0];
        section->Day = std::stoi(tokens[1]);
        section->ShiftID = tokens[2];
        section->Weight = std::stoi(tokens[3]);
    }
}

void InstanceParser::addCover(Cover* section, const std::string& s, const std::string& delimiter) {
    auto tokens = tokenize(s, ",");
    
    if (tokens.size() >= 5) {
        section->Day = std::stoi(tokens[0]);
        section->ShiftID = tokens[1];
        section->Requirement = std::stoi(tokens[2]);
        section->Weight_for_under = std::stoi(tokens[3]);
        section->Weight_for_over = std::stoi(tokens[4]);
    }
}

bool InstanceParser::parseInstance(const std::string& filename,
                                  int& horizon,
                                  std::vector<Staff>& workers,
                                  std::vector<Shift>& shifts,
                                  std::vector<DaysOff>& days_off,
                                  std::vector<ShiftOnRequest>& shift_on_requests,
                                  std::vector<ShiftOffRequest>& shift_off_requests,
                                  std::vector<Cover>& cover_requirements) {
    
    std::ifstream fileStream(filename);
    if (!fileStream.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }
    
    // Clear all containers
    workers.clear();
    shifts.clear();
    days_off.clear();
    shift_on_requests.clear();
    shift_off_requests.clear();
    cover_requirements.clear();
    
    bool copying = false;
    int current_section = 0;
    std::string line;
    
    while (std::getline(fileStream, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Always check for section headers first
        bool is_section_header = false;
        for (int i = 0; i < 7; i++) {
            if (line == sections[i]) {
                current_section = i;
                copying = true;
                is_section_header = true;
                break;
            }
        }
        
        if (is_section_header) {
            continue;
        } else if (line.length() > 1 && copying) {
            // Process section content
            switch (current_section) {
                case 0: // SECTION_HORIZON
                    horizon = std::stoi(line);
                    break;
                    
                case 1: { // SECTION_SHIFTS
                    Shift shift;
                    addShift(&shift, line, ",");
                    shifts.push_back(shift);
                    break;
                }
                
                case 2: { // SECTION_STAFF
                    Staff staff;
                    addStaff(&staff, line, ",");
                    workers.push_back(staff);
                    break;
                }
                
                case 3: { // SECTION_DAYS_OFF
                    DaysOff day_off;
                    addDaysOff(&day_off, line, ",");
                    days_off.push_back(day_off);
                    break;
                }
                
                case 4: { // SECTION_SHIFT_ON_REQUESTS
                    ShiftOnRequest request;
                    addShiftOnRequest(&request, line, ",");
                    shift_on_requests.push_back(request);
                    break;
                }
                
                case 5: { // SECTION_SHIFT_OFF_REQUESTS
                    ShiftOffRequest request;
                    addShiftOffRequest(&request, line, ",");
                    shift_off_requests.push_back(request);
                    break;
                }
                
                case 6: { // SECTION_COVER
                    Cover cover;
                    addCover(&cover, line, ",");
                    cover_requirements.push_back(cover);
                    break;
                }
            }
        } else if (line.length() <= 1 && copying) {
            copying = false;
        }
    }
    
    fileStream.close();
    return true;
}