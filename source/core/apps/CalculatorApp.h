#pragma once
#include "../PhoneApp.h"
#include <string>
#include <vector>

struct CalcHistoryItem {
    std::string expression; // e.g. "12 + 8 ="
    double result;          // e.g. 20.0
    std::string resultStr;  // e.g. "20"
};

class CalculatorApp : public PhoneApp {
private:
    std::string m_currentInput;
    double m_firstOperand;
    std::string m_pendingOperator;
    bool m_isNewInput;
    bool m_hasDecimal;

    // History state
    std::vector<CalcHistoryItem> m_history;
    bool m_showHistory;

    // Helper functions for calculation
    void clearAll();
    void clearEntry();
    void inputDigit(char digit);
    void inputDecimal();
    void setOperator(const std::string& op);
    void calculate();
    void toggleSign();
    void applyPercentage();
    void backspace();
    void addHistoryItem(double op1, const std::string& op, double op2, double res);
    std::string formatDisplayValue(double val);

    // UI rendering helpers
    void drawHistoryView();

public:
    CalculatorApp();
    void onOpen() override;
    void onDraw() override;
    void onLanguageChange() override { name = TR("calculator.title"); }
    std::string getAppDescription() const override { return TR("appstore.desc.calculator"); }
};
