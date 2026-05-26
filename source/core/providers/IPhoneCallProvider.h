#pragma once
#include <string>

class IPhoneCallProvider {
public:
    virtual ~IPhoneCallProvider() = default;
    virtual bool IsIncomingCallActive() = 0;
    virtual std::string GetCallerId() = 0;
    virtual void AnswerCall() = 0;
    virtual void HangUpCall() = 0;
};
