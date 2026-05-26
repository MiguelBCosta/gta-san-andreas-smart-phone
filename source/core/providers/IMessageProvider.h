#pragma once
#include <string>

class IMessageProvider {
public:
    virtual ~IMessageProvider() = default;
    
    // Returns the ID/name of the mission completed in the current frame (e.g. "ryder1"), or empty string.
    virtual std::string GetCompletedMissionThisFrame() = 0;
    
    // Resets the completed mission flag/ID once the app processes it.
    virtual void ResetCompletedMission() = 0;
};
