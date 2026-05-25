#pragma once
#include <string>

class IStorageProvider {
public:
    virtual ~IStorageProvider() = default;
    
    // Reads JSON content from the given slot as a string. Returns empty string if slot doesn't exist or is empty.
    virtual std::string ReadSlotData(int slot) = 0;
    
    // Writes JSON content string to the given slot. Returns true if successful.
    virtual bool WriteSlotData(int slot, const std::string& jsonContent) = 0;
    
    // Clears/deletes data for the given slot. Returns true if successful.
    virtual bool DeleteSlotData(int slot) = 0;
};
