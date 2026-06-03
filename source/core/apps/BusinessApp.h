#pragma once
#include "../PhoneApp.h"
#include "../providers/IBusinessProvider.h"
#include "../providers/IStatsProvider.h"
#include "../ServiceContainer.h"
#include <IconsFontAwesome5.h>
#include <string>
#include <vector>

class BusinessApp : public PhoneApp {
private:
    Inject<IBusinessProvider> m_businessProvider;
    Inject<IStatsProvider> m_statsProvider;
    
    std::vector<BusinessInfo> m_cachedBusinesses;
    
    // Feedback text for buying or error states
    std::string m_feedbackMsg = "";
    float m_feedbackTimer = 0.0f;

    void SetFeedback(const std::string& msg, float duration = 2.0f);
    void RefreshData();

public:
    BusinessApp();

    void onOpen() override;
    void onClose() override;
    void update(float dt) override;
    void onDraw() override;
    void onLanguageChange() override { name = TR("business.title"); }
    std::string getAppDescription() const override { return TR("appstore.desc.business"); }
};
