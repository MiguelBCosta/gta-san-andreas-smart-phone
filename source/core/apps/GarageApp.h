#pragma once
#include "../PhoneApp.h"
#include "../providers/IGarageProvider.h"
#include "../ServiceContainer.h"
#include <vector>
#include <string>

class GarageApp : public PhoneApp {
private:
    struct SavedVehicle {
        VehicleInfo info;
        int deliveryId = -1; // runtime only
    };

    Inject<IGarageProvider> m_provider;
    std::vector<SavedVehicle> m_vehicles;
    int m_selectedIdx = -1;
    int m_renamingIdx = -1;
    int m_editingPlateIdx = -1;
    char m_renameBuf[256] = "";
    char m_plateBuf[16] = "";

    void DrawDetailsScreen();
    void DrawListScreen();

public:
    GarageApp();

    void onOpen() override;
    void onClose() override;
    void update(float dt) override;
    bool onBack() override;
    void onSave(nlohmann::json& out) override;
    void onLoad(const nlohmann::json& in) override;
    void onWipe() override;
    void onDraw() override;
    void onLanguageChange() override { name = TR("garage.title"); }
};
