#ifndef INSTANCE_PARSER_H
#define INSTANCE_PARSER_H

#include "data_structures.h"
#include <string>
#include <vector>

/**
 * Handles parsing of NSP instance files
 */
class InstanceParser {
private:
    // Parsing helper functions (extracted from original main.cpp)
    std::vector<std::string> tokenize(const std::string& s, const std::string& delimiter = " ");
    std::vector<std::string> cantFollow(const std::string& s, const std::string& delimiter = " ");
    std::string parseShift(const std::string& s, const std::string& delimiter = " ");
    std::vector<std::string> maxshift(const std::string& s, const std::string& delimiter = " ");
    
    void addShift(Shift* shift, const std::string& s, const std::string& delimiter = " ");
    void addStaff(Staff* staff, const std::string& s, const std::string& delimiter = " ");
    void addDaysOff(DaysOff* section, const std::string& s, const std::string& delimiter = "");
    void addShiftOnRequest(ShiftOnRequest* section, const std::string& s, const std::string& delimiter = "");
    void addShiftOffRequest(ShiftOffRequest* section, const std::string& s, const std::string& delimiter = "");
    void addCover(Cover* section, const std::string& s, const std::string& delimiter = "");
    
public:
    /**
     * Parse an instance file and populate the data structures
     */
    bool parseInstance(const std::string& filename,
                      int& horizon,
                      std::vector<Staff>& workers,
                      std::vector<Shift>& shifts,
                      std::vector<DaysOff>& days_off,
                      std::vector<ShiftOnRequest>& shift_on_requests,
                      std::vector<ShiftOffRequest>& shift_off_requests,
                      std::vector<Cover>& cover_requirements);
};

#endif // INSTANCE_PARSER_H