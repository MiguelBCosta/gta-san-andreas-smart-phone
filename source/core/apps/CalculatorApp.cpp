#include "CalculatorApp.h"
#include <imgui.h>
#include <cmath>
#include <cstdio>
#include <string>
#include <IconsFontAwesome5.h>

// Helper to draw custom styled buttons
static bool CalcButton(const char* label, ImVec2 size, ImVec4 baseColor, ImVec4 hoverColor, ImVec4 activeColor, ImVec4 textColor, ImVec2 align = ImVec2(0.5f, 0.5f)) {
    ImGui::PushStyleColor(ImGuiCol_Button, baseColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeColor);
    ImGui::PushStyleColor(ImGuiCol_Text, textColor);
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, align);
    
    bool pressed = ImGui::Button(label, size);
    
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);
    return pressed;
}

// Helper to rotate vertices in ImDrawList (e.g. to flip icons)
static void RotateVertices(ImDrawList* drawList, int vtxStart, ImVec2 center, float angle) {
    float s = sinf(angle);
    float c = cosf(angle);
    for (int i = vtxStart; i < drawList->VtxBuffer.Size; i++) {
        ImDrawVert& v = drawList->VtxBuffer[i];
        float x = v.pos.x - center.x;
        float y = v.pos.y - center.y;
        v.pos.x = center.x + (x * c - y * s);
        v.pos.y = center.y + (x * s + y * c);
    }
}

CalculatorApp::CalculatorApp() {
    id = "calculator";
    icon = ICON_FA_CALCULATOR;
    name = "Calculadora";
    color = ImVec4(0.12f, 0.12f, 0.14f, 1.0f); // Sleek iOS-like dark bg icon
    m_showHistory = false;
    clearAll();
}

void CalculatorApp::onOpen() {
    clearAll();
    m_showHistory = false;
}

void CalculatorApp::clearAll() {
    m_currentInput = "0";
    m_firstOperand = 0.0;
    m_pendingOperator = "";
    m_isNewInput = true;
    m_hasDecimal = false;
}

void CalculatorApp::clearEntry() {
    m_currentInput = "0";
    m_isNewInput = true;
    m_hasDecimal = false;
}

void CalculatorApp::inputDigit(char digit) {
    if (m_isNewInput) {
        m_currentInput = "";
        m_isNewInput = false;
        m_hasDecimal = false;
    }
    
    if (m_currentInput == "0") {
        m_currentInput = "";
    }
    
    // Max 9 digits limit (excluding sign and decimals)
    int digitCount = 0;
    for (char c : m_currentInput) {
        if (c >= '0' && c <= '9') {
            digitCount++;
        }
    }
    if (digitCount >= 9) return;
    
    m_currentInput += digit;
}

void CalculatorApp::inputDecimal() {
    if (m_isNewInput) {
        m_currentInput = "0.";
        m_isNewInput = false;
        m_hasDecimal = true;
        return;
    }
    
    if (!m_hasDecimal) {
        m_currentInput += ".";
        m_hasDecimal = true;
    }
}

void CalculatorApp::setOperator(const std::string& op) {
    if (!m_pendingOperator.empty() && !m_isNewInput) {
        calculate();
    }
    
    try {
        m_firstOperand = std::stod(m_currentInput);
    } catch (...) {
        m_firstOperand = 0.0;
    }
    
    m_pendingOperator = op;
    m_isNewInput = true;
}

void CalculatorApp::calculate() {
    if (m_pendingOperator.empty()) return;
    
    double secondOperand;
    try {
        secondOperand = std::stod(m_currentInput);
    } catch (...) {
        secondOperand = 0.0;
    }
    
    double result = 0.0;
    std::string op = m_pendingOperator;
    
    if (op == "+") {
        result = m_firstOperand + secondOperand;
    } else if (op == "-") {
        result = m_firstOperand - secondOperand;
    } else if (op == "*") {
        result = m_firstOperand * secondOperand;
    } else if (op == "/") {
        if (secondOperand == 0.0) {
            m_currentInput = "Erro";
            m_pendingOperator = "";
            m_isNewInput = true;
            m_hasDecimal = false;
            return;
        }
        result = m_firstOperand / secondOperand;
    }
    
    // Save to history list
    addHistoryItem(m_firstOperand, op, secondOperand, result);
    
    m_currentInput = formatDisplayValue(result);
    m_firstOperand = result;
    m_pendingOperator = "";
    m_isNewInput = true;
    m_hasDecimal = (m_currentInput.find('.') != std::string::npos);
}

void CalculatorApp::toggleSign() {
    if (m_currentInput == "0" || m_currentInput == "Erro") return;
    
    if (m_currentInput[0] == '-') {
        m_currentInput = m_currentInput.substr(1);
    } else {
        m_currentInput = "-" + m_currentInput;
    }
}

void CalculatorApp::applyPercentage() {
    double val;
    try {
        val = std::stod(m_currentInput);
    } catch (...) {
        val = 0.0;
    }
    
    double prevVal = val;
    val /= 100.0;
    m_currentInput = formatDisplayValue(val);
    m_isNewInput = true;
    m_hasDecimal = (m_currentInput.find('.') != std::string::npos);
    
    // Add percentage to history as unary op: e.g. "50 % =" -> "0.5"
    addHistoryItem(prevVal, "%", 0.0, val); // We pass 0.0 for op2, addHistoryItem handles display
}

void CalculatorApp::backspace() {
    if (m_isNewInput) return; // Ignore if it's showing a final calculation result
    if (m_currentInput == "Erro" || m_currentInput == "0" || m_currentInput.empty()) {
        m_currentInput = "0";
        return;
    }
    
    m_currentInput.pop_back();
    
    // If it becomes empty, or just negative sign, reset to "0"
    if (m_currentInput.empty() || m_currentInput == "-") {
        m_currentInput = "0";
    }
    
    m_hasDecimal = (m_currentInput.find('.') != std::string::npos);
}

void CalculatorApp::addHistoryItem(double op1, const std::string& op, double op2, double res) {
    CalcHistoryItem item;
    
    if (op == "%") {
        item.expression = formatDisplayValue(op1) + " % =";
    } else {
        item.expression = formatDisplayValue(op1) + " " + op + " " + formatDisplayValue(op2) + " =";
    }
    
    item.result = res;
    item.resultStr = formatDisplayValue(res);
    
    // Prevent duplicated items at the very top of history
    if (!m_history.empty() && m_history.back().expression == item.expression && m_history.back().resultStr == item.resultStr) {
        return;
    }
    
    if (m_history.size() >= 50) {
        m_history.erase(m_history.begin());
    }
    m_history.push_back(item);
}

std::string CalculatorApp::formatDisplayValue(double val) {
    if (std::isnan(val) || std::isinf(val)) {
        return "Erro";
    }
    
    // Whole number check within representation limits
    if (val >= -999999999.0 && val <= 999999999.0) {
        double intPart;
        if (std::modf(val, &intPart) == 0.0) {
            return std::to_string((long long)val);
        }
    }
    
    // Format to a string with high precision
    char buf[64];
    sprintf_s(buf, sizeof(buf), "%.9g", val);
    std::string s(buf);
    
    // If scientific notation is used, return as is
    if (s.find('e') != std::string::npos || s.find('E') != std::string::npos) {
        return s;
    }
    
    // Clean trailing zeros
    if (s.find('.') != std::string::npos) {
        while (s.back() == '0') {
            s.pop_back();
        }
        if (s.back() == '.') {
            s.pop_back();
        }
    }
    
    return s;
}

void CalculatorApp::drawHistoryView() {
    ImGui::Text(ICON_FA_HISTORY " Histórico de Cálculos");
    ImGui::Separator();
    ImGui::Spacing();
    
    // Scrollable List of History items (leaves 40px at the bottom for navigation buttons)
    ImGui::BeginChild("##history_scroll", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 45.0f), false);
    
    if (m_history.empty()) {
        ImGui::SetCursorPosY(ImGui::GetContentRegionAvail().y / 2.0f - 10.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 0.8f));
        float textW = ImGui::CalcTextSize("Nenhum histórico disponível").x;
        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - textW) / 2.0f);
        ImGui::Text("Nenhum histórico disponível");
        ImGui::PopStyleColor();
    } else {
        // Draw items in reverse chronological order (latest first)
        for (int i = (int)m_history.size() - 1; i >= 0; i--) {
            const auto& item = m_history[i];
            
            ImGui::PushID(i);
            
            // Custom stylings for history cards
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.12f, 0.12f, 0.16f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.12f, 0.12f, 0.16f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.12f, 0.12f, 0.16f, 0.7f));
            
            std::string btnId = std::string("##hist_btn_") + std::to_string(i);
            if (ImGui::Button(btnId.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 54.0f))) {
                // Restore result to continue calculation
                m_currentInput = item.resultStr;
                m_firstOperand = item.result;
                m_pendingOperator = "";
                m_isNewInput = true;
                m_hasDecimal = (m_currentInput.find('.') != std::string::npos);
                m_showHistory = false;
            }
            
            // Draw card texts manually inside button rect
            ImVec2 rMin = ImGui::GetItemRectMin();
            ImVec2 rMax = ImGui::GetItemRectMax();
            ImDrawList* draw = ImGui::GetWindowDrawList();
            
            // Expression text (grey, small, right aligned)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.55f, 0.55f, 1.0f));
            ImGui::SetWindowFontScale(0.85f);
            float exprW = ImGui::CalcTextSize(item.expression.c_str()).x;
            draw->AddText(ImVec2(rMax.x - exprW - 10.0f, rMin.y + 6.0f), ImGui::GetColorU32(ImGuiCol_Text), item.expression.c_str());
            ImGui::SetWindowFontScale(1.0f);
            ImGui::PopStyleColor();
            
            // Result text (orange/white, larger, right aligned)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.62f, 0.082f, 1.0f));
            ImGui::SetWindowFontScale(1.15f);
            float resW = ImGui::CalcTextSize(item.resultStr.c_str()).x;
            draw->AddText(ImVec2(rMax.x - resW - 10.0f, rMin.y + 24.0f), ImGui::GetColorU32(ImGuiCol_Text), item.resultStr.c_str());
            ImGui::SetWindowFontScale(1.0f);
            ImGui::PopStyleColor();
            
            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar();
            ImGui::Spacing();
            
            ImGui::PopID();
        }
    }
    
    ImGui::EndChild();
    
    // Bottom Buttons (Clear & Back)
    ImGui::Separator();
    ImGui::Spacing();
    
    float availX = ImGui::GetContentRegionAvail().x;
    float btnW = (availX - 10.0f) / 2.0f;
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.25f, 0.12f, 0.12f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.15f, 0.15f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.45f, 0.18f, 0.18f, 1.0f));
    if (ImGui::Button("Limpar", ImVec2(btnW, 30.0f))) {
        m_history.clear();
    }
    ImGui::PopStyleColor(3);
    
    ImGui::SameLine();
    
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.18f, 0.18f, 0.25f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.28f, 0.38f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.38f, 0.38f, 0.48f, 1.0f));
    if (ImGui::Button("Voltar", ImVec2(btnW, 30.0f))) {
        m_showHistory = false;
    }
    ImGui::PopStyleColor(3);
    
    ImGui::PopStyleVar();
}

void CalculatorApp::onDraw() {
    // Check physical backspace
    if (ImGui::IsKeyPressed(ImGuiKey_Backspace, false)) {
        backspace();
    }
    
    // Render History screen instead of standard calculator if toggled
    if (m_showHistory) {
        drawHistoryView();
        return;
    }
    
    // ---- Small Navigation / Utility Header ----
    ImGui::SetCursorPosY(8.0f); // Move up close to the separator line
    
    // History clock button
    ImGui::SetCursorPosX(8.0f);
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Text,           ImVec4(0.7f, 0.7f, 0.75f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    
    if (ImGui::Button(ICON_FA_HISTORY " Histórico", ImVec2(90.0f, 25.0f))) {
        m_showHistory = true;
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);
    
    // ---- Display Area ----
    // Adjust scale based on length of input text to prevent overflow
    float scale = 3.3f;
    size_t len = m_currentInput.length();
    if (len > 6) {
        scale = 3.3f - 0.23f * (len - 6);
        if (scale < 1.4f) scale = 1.4f;
    }
    
    float displayY = 75.0f;
    
    // Render Display text (right-aligned)
    ImGui::SetCursorPosY(displayY);
    ImGui::SetWindowFontScale(scale);
    float textWidth = ImGui::CalcTextSize(m_currentInput.c_str()).x;
    float textHeight = ImGui::CalcTextSize(m_currentInput.c_str()).y;
    ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - textWidth - 8.0f);
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::Text("%s", m_currentInput.c_str());
    ImGui::PopStyleColor();
    ImGui::SetWindowFontScale(1.0f); // Restore base scale
    
    // Backspace button (left aligned, same vertical level as the display numbers)
    if (!m_isNewInput && m_currentInput != "0" && m_currentInput != "Erro") {
        float btnHeight = 30.0f;
        float btnWidth = 35.0f;
        // Vertically center the backspace button relative to the display number text height
        float btnY = displayY + (textHeight - btnHeight) * 0.5f;
        
        ImGui::SetCursorPosY(btnY);
        ImGui::SetCursorPosX(8.0f);
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_Text,           ImVec4(1.0f, 1.0f, 1.0f, 0.85f)); // Sleek white backspace icon
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        
        ImGui::SetWindowFontScale(1.15f); // Make icon slightly larger
        
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        int vtxStart = drawList->VtxBuffer.Size;
        
        bool clicked = ImGui::Button(ICON_FA_BACKSPACE, ImVec2(btnWidth, btnHeight));
        
        // Rotate 180 degrees (PI radians) around the button center to point right
        ImVec2 itemMin = ImGui::GetItemRectMin();
        ImVec2 center = ImVec2(itemMin.x + btnWidth * 0.5f, itemMin.y + btnHeight * 0.5f);
        RotateVertices(drawList, vtxStart, center, 3.14159265f);
        
        if (clicked) {
            backspace();
        }
        ImGui::SetWindowFontScale(1.0f);
        
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);
    }
    
    // ---- Keyboard Layout ----
    float availW = ImGui::GetContentRegionAvail().x;
    float gap = 11.0f;
    float btnSz = (availW - 3.0f * gap) / 4.0f;
    float buttonsHeight = 5.0f * btnSz + 4.0f * gap;
    
    // Pinned to the bottom
    float buttonsY = ImGui::GetWindowHeight() - buttonsHeight - 12.0f;
    ImGui::SetCursorPosY(buttonsY);
    
    // iOS Button Colors
    ImVec4 cLightGray(0.647f, 0.647f, 0.647f, 1.0f);
    ImVec4 cLightGrayHover(0.75f, 0.75f, 0.75f, 1.0f);
    ImVec4 cLightGrayActive(0.85f, 0.85f, 0.85f, 1.0f);
    ImVec4 cBlack(0.0f, 0.0f, 0.0f, 1.0f);
    
    ImVec4 cOrange(1.0f, 0.62f, 0.082f, 1.0f);
    ImVec4 cOrangeHover(1.0f, 0.72f, 0.2f, 1.0f);
    ImVec4 cOrangeActive(1.0f, 0.8f, 0.35f, 1.0f);
    ImVec4 cWhite(1.0f, 1.0f, 1.0f, 1.0f);
    
    ImVec4 cDarkGray(0.20f, 0.20f, 0.20f, 1.0f);
    ImVec4 cDarkGrayHover(0.30f, 0.30f, 0.30f, 1.0f);
    ImVec4 cDarkGrayActive(0.40f, 0.40f, 0.40f, 1.0f);
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, btnSz * 0.5f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(gap, gap));
    ImGui::SetWindowFontScale(1.4f); // Slightly larger font for keys
    
    // Row 1: AC/C | +/- | % | /
    bool isClearEntry = (m_currentInput != "0" && m_currentInput != "");
    const char* clearLabel = isClearEntry ? "C" : "AC";
    if (CalcButton(clearLabel, ImVec2(btnSz, btnSz), cLightGray, cLightGrayHover, cLightGrayActive, cBlack)) {
        if (isClearEntry) clearEntry();
        else clearAll();
    }
    
    ImGui::SameLine();
    if (CalcButton("+/-", ImVec2(btnSz, btnSz), cLightGray, cLightGrayHover, cLightGrayActive, cBlack)) {
        toggleSign();
    }
    
    ImGui::SameLine();
    if (CalcButton("%", ImVec2(btnSz, btnSz), cLightGray, cLightGrayHover, cLightGrayActive, cBlack)) {
        applyPercentage();
    }
    
    ImGui::SameLine();
    bool isDivActive = (m_pendingOperator == "/" && m_isNewInput);
    if (CalcButton("/", ImVec2(btnSz, btnSz), isDivActive ? cWhite : cOrange, isDivActive ? cWhite : cOrangeHover, cOrangeActive, isDivActive ? cOrange : cWhite)) {
        setOperator("/");
    }
    
    // Row 2: 7 | 8 | 9 | *
    if (CalcButton("7", ImVec2(btnSz, btnSz), cDarkGray, cDarkGrayHover, cDarkGrayActive, cWhite)) {
        inputDigit('7');
    }
    
    ImGui::SameLine();
    if (CalcButton("8", ImVec2(btnSz, btnSz), cDarkGray, cDarkGrayHover, cDarkGrayActive, cWhite)) {
        inputDigit('8');
    }
    
    ImGui::SameLine();
    if (CalcButton("9", ImVec2(btnSz, btnSz), cDarkGray, cDarkGrayHover, cDarkGrayActive, cWhite)) {
        inputDigit('9');
    }
    
    ImGui::SameLine();
    bool isMulActive = (m_pendingOperator == "*" && m_isNewInput);
    if (CalcButton("x", ImVec2(btnSz, btnSz), isMulActive ? cWhite : cOrange, isMulActive ? cWhite : cOrangeHover, cOrangeActive, isMulActive ? cOrange : cWhite)) {
        setOperator("*");
    }
    
    // Row 3: 4 | 5 | 6 | -
    if (CalcButton("4", ImVec2(btnSz, btnSz), cDarkGray, cDarkGrayHover, cDarkGrayActive, cWhite)) {
        inputDigit('4');
    }
    
    ImGui::SameLine();
    if (CalcButton("5", ImVec2(btnSz, btnSz), cDarkGray, cDarkGrayHover, cDarkGrayActive, cWhite)) {
        inputDigit('5');
    }
    
    ImGui::SameLine();
    if (CalcButton("6", ImVec2(btnSz, btnSz), cDarkGray, cDarkGrayHover, cDarkGrayActive, cWhite)) {
        inputDigit('6');
    }
    
    ImGui::SameLine();
    bool isSubActive = (m_pendingOperator == "-" && m_isNewInput);
    if (CalcButton("-", ImVec2(btnSz, btnSz), isSubActive ? cWhite : cOrange, isSubActive ? cWhite : cOrangeHover, cOrangeActive, isSubActive ? cOrange : cWhite)) {
        setOperator("-");
    }
    
    // Row 4: 1 | 2 | 3 | +
    if (CalcButton("1", ImVec2(btnSz, btnSz), cDarkGray, cDarkGrayHover, cDarkGrayActive, cWhite)) {
        inputDigit('1');
    }
    
    ImGui::SameLine();
    if (CalcButton("2", ImVec2(btnSz, btnSz), cDarkGray, cDarkGrayHover, cDarkGrayActive, cWhite)) {
        inputDigit('2');
    }
    
    ImGui::SameLine();
    if (CalcButton("3", ImVec2(btnSz, btnSz), cDarkGray, cDarkGrayHover, cDarkGrayActive, cWhite)) {
        inputDigit('3');
    }
    
    ImGui::SameLine();
    bool isAddActive = (m_pendingOperator == "+" && m_isNewInput);
    if (CalcButton("+", ImVec2(btnSz, btnSz), isAddActive ? cWhite : cOrange, isAddActive ? cWhite : cOrangeHover, cOrangeActive, isAddActive ? cOrange : cWhite)) {
        setOperator("+");
    }
    
    // Row 5: 0 | , | =
    // 0 is double-width
    float zeroW = 2.0f * btnSz + gap;
    if (CalcButton("0", ImVec2(zeroW, btnSz), cDarkGray, cDarkGrayHover, cDarkGrayActive, cWhite, ImVec2(0.25f, 0.5f))) {
        inputDigit('0');
    }
    
    ImGui::SameLine();
    if (CalcButton(",", ImVec2(btnSz, btnSz), cDarkGray, cDarkGrayHover, cDarkGrayActive, cWhite)) {
        inputDecimal();
    }
    
    ImGui::SameLine();
    if (CalcButton("=", ImVec2(btnSz, btnSz), cOrange, cOrangeHover, cOrangeActive, cWhite)) {
        calculate();
    }
    
    ImGui::SetWindowFontScale(1.0f); // Restore base scale
    ImGui::PopStyleVar(2);
}
