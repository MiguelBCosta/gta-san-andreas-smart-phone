#pragma once

struct PhoneTime {
    int hours;
    int minutes;
    int seconds;
    int dayOfWeek; // 1-7 (1 = Sunday)
};

class IClockProvider {
public:
    virtual ~IClockProvider() = default;
    virtual PhoneTime GetTime() = 0;
    virtual void SkipTime(int hoursToPass) = 0;
    virtual bool CanSkipTime() = 0;
};
