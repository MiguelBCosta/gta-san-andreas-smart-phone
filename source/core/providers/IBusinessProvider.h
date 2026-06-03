#pragma once
#include <string>
#include <vector>

struct BusinessInfo {
    std::string id;
    std::string name;
    std::string description;
    std::string icon;
    int cost;
    int maxProfit;
    int dailyYield;
    bool owned;
    int currentProfit;
    bool unlocked = true;
};

class IBusinessProvider {
public:
    virtual ~IBusinessProvider() = default;
    virtual std::vector<BusinessInfo> GetBusinesses() = 0;
    virtual void CollectProfit(const std::string& id) = 0;
    virtual void CollectAllProfits() = 0;
    virtual void Update(float dt) {}
    virtual bool CanPurchaseInApp() { return false; }
    virtual void PurchaseBusiness(const std::string& id) {}
};

