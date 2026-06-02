#include "ProfileApp.h"
#include "../Phone.h"
#include "../providers/IAvatarProvider.h"
#include "../providers/IStatsProvider.h"
#include "../providers/IWallpaperProvider.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

extern Phone phone;

// Helper function to draw inverse corners for circular masking
static void DrawInverseCorner(ImDrawList *draw, ImVec2 center, float radius,
                              int corner_idx, ImU32 color) {
  const int num_segments = 8;
  ImVec2 pts[num_segments + 3];

  ImVec2 corner_pos;
  float start_angle = 0.0f;
  const float pi = 3.1415926535f;

  if (corner_idx == 0) { // Top-Left
    corner_pos = ImVec2(center.x - radius, center.y - radius);
    start_angle = pi;           // 180 deg to 270 deg
  } else if (corner_idx == 1) { // Top-Right
    corner_pos = ImVec2(center.x + radius, center.y - radius);
    start_angle = 1.5f * pi;    // 270 deg to 360 deg
  } else if (corner_idx == 2) { // Bottom-Right
    corner_pos = ImVec2(center.x + radius, center.y + radius);
    start_angle = 0.0f; // 0 deg to 90 deg
  } else {              // Bottom-Left
    corner_pos = ImVec2(center.x - radius, center.y + radius);
    start_angle = 0.5f * pi; // 90 deg to 180 deg
  }

  pts[0] = corner_pos;
  for (int i = 0; i <= num_segments; ++i) {
    float angle = start_angle + (float)i / (float)num_segments * (0.5f * pi);
    pts[i + 1] = ImVec2(center.x + cosf(angle) * radius,
                        center.y + sinf(angle) * radius);
  }

  draw->AddConvexPolyFilled(pts, num_segments + 2, color);
}

static void DrawRoundedScreenMask(ImDrawList *draw, ImVec2 pMin, ImVec2 pMax,
                                  float radius, ImU32 color) {
  DrawInverseCorner(draw, ImVec2(pMin.x + radius, pMin.y + radius), radius, 0,
                    color);
  DrawInverseCorner(draw, ImVec2(pMax.x - radius, pMin.y + radius), radius, 1,
                    color);
  DrawInverseCorner(draw, ImVec2(pMax.x - radius, pMax.y - radius), radius, 2,
                    color);
  DrawInverseCorner(draw, ImVec2(pMin.x + radius, pMax.y - radius), radius, 3,
                    color);
}

ProfileApp::ProfileApp() {
  id = "profile";
  icon = ICON_FA_USER;
  name = "Perfil";
  color = ImVec4(0.0f, 0.478f, 1.0f, 1.0f); // Apple blue
}

void ProfileApp::onOpen() { m_activeSubmenu = 0; }

void ProfileApp::onClose() {}

bool ProfileApp::onBack() {
  if (m_activeSubmenu > 0) {
    m_activeSubmenu = 0;
    return true; // Handled back button press
  }
  return false; // Close the app
}

void ProfileApp::onDraw() {
  Inject<IStatsProvider> stats;
  if (!stats.isValid()) {
    ImGui::TextColored(ImVec4(1, 0, 0, 1),
                       "Erro: Provedor de stats indisponivel.");
    return;
  }

  if (m_activeSubmenu == 0) {
    drawMainMenu();
  } else if (m_activeSubmenu == 1) {
    drawJogadorMenu();
  } else if (m_activeSubmenu == 2) {
    drawHabilidadesMenu();
  } else if (m_activeSubmenu == 3) {
    drawGangMenu();
  } else if (m_activeSubmenu == 4) {
    drawCrimesMenu();
  } else if (m_activeSubmenu == 5) {
    drawProgressoMenu();
  }
}

void ProfileApp::drawMainMenu() {
  ImGui::Spacing();

  // 1. Render CJ Avatar circular
  Inject<IAvatarProvider> ap;
  ImTextureID avatarTex = 0;
  if (ap.isValid()) {
    avatarTex = ap->GetAvatarTexture("profile");
  }

  float avatarSize = 74.0f;
  ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - avatarSize) / 2.0f);
  ImVec2 pos = ImGui::GetCursorScreenPos();
  ImDrawList *drawList = ImGui::GetWindowDrawList();

  if (avatarTex) {
    drawList->AddImage(avatarTex, pos,
                       ImVec2(pos.x + avatarSize, pos.y + avatarSize));
  } else {
    // Fallback placeholder grey circle
    drawList->AddRectFilled(pos, ImVec2(pos.x + avatarSize, pos.y + avatarSize),
                            IM_COL32(80, 80, 85, 255), avatarSize / 2.0f);
  }

  // Apply circular rounding mask matching background color
  ImU32 bgColor =
      ImGui::ColorConvertFloat4ToU32(ImVec4(0.05f, 0.05f, 0.07f, 0.97f));
  DrawRoundedScreenMask(drawList, pos,
                        ImVec2(pos.x + avatarSize, pos.y + avatarSize),
                        avatarSize / 2.0f, bgColor);

  // Silver thin border
  drawList->AddCircle(
      ImVec2(pos.x + avatarSize / 2.0f, pos.y + avatarSize / 2.0f),
      avatarSize / 2.0f, IM_COL32(255, 255, 255, 60), 32, 2.0f);

  ImGui::Dummy(ImVec2(avatarSize, avatarSize + 8.0f));

  // Name
  ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
  ImGui::SetWindowFontScale(1.4f);
  const char *charName = "CJ (Carl Johnson)";
  float nw = ImGui::CalcTextSize(charName).x;
  ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - nw) / 2.0f);
  ImGui::Text("%s", charName);
  ImGui::SetWindowFontScale(1.0f);
  ImGui::PopFont();

  ImGui::Spacing();
  ImGui::Spacing();

  // Grouped Rows layout for Submenus
  ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
  ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.18f, 0.18f, 0.20f, 0.8f));

  float rowHeight = 44.0f;
  float menuHeight = 5 * rowHeight;

  ImGui::BeginChild("##submenus_list",
                    ImVec2(ImGui::GetContentRegionAvail().x, menuHeight), true,
                    ImGuiWindowFlags_NoScrollbar);

  auto drawRow = [&](int idSub, const char *icon, const char *label,
                     ImVec4 color) {
    ImGui::PushID(idSub);

    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1, 1, 1, 0.05f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(1, 1, 1, 0.1f));

    if (ImGui::Selectable(
            "##select_row", false, ImGuiSelectableFlags_None,
            ImVec2(ImGui::GetContentRegionAvail().x, rowHeight - 2.0f))) {
      m_activeSubmenu = idSub;
    }
    ImGui::PopStyleColor(3);

    // Content
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - rowHeight +
                         (rowHeight - ImGui::GetTextLineHeight()) / 2.0f);
    ImGui::Indent(12.0f);

    // Icon
    ImGui::TextColored(color, "%s", icon);
    ImGui::SameLine();
    ImGui::Text("%s", label);

    // Chevron
    ImGui::SameLine(ImGui::GetWindowWidth() - 36.0f);
    ImGui::TextDisabled(ICON_FA_CHEVRON_RIGHT);

    ImGui::Unindent(12.0f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() +
                         (rowHeight - ImGui::GetTextLineHeight()) / 2.0f);

    if (idSub < 5) {
      ImVec2 nextCursor = ImGui::GetCursorScreenPos();
      ImGui::GetWindowDrawList()->AddLine(
          ImVec2(nextCursor.x + 36.0f, nextCursor.y),
          ImVec2(nextCursor.x + ImGui::GetContentRegionAvail().x, nextCursor.y),
          IM_COL32(255, 255, 255, 20));
      ImGui::Spacing();
    } else {
      ImGui::Dummy(ImVec2(0.0f, 0.0f));
    }

    ImGui::PopID();
  };

  drawRow(1, ICON_FA_USER, "Jogador", ImVec4(0.0f, 0.478f, 1.0f, 1.0f)); // Blue
  drawRow(2, ICON_FA_AWARD, "Habilidades",
          ImVec4(1.0f, 0.584f, 0.0f, 1.0f)); // Orange
  drawRow(3, ICON_FA_USERS, "Gang & Respeito",
          ImVec4(0.196f, 0.843f, 0.294f, 1.0f)); // Green
  drawRow(4, ICON_FA_SHIELD_ALT, "Crimes & Wanted",
          ImVec4(1.0f, 0.231f, 0.188f, 1.0f)); // Red
  drawRow(5, ICON_FA_CHART_LINE, "Progresso",
          ImVec4(0.686f, 0.322f, 0.871f, 1.0f)); // Purple

  ImGui::EndChild();
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();
}

void ProfileApp::drawStatRow(const char *label, const char *value,
                             const char *icon, ImVec4 iconColor) {
  ImGui::Spacing();
  if (icon) {
    ImGui::TextColored(iconColor, "%s", icon);
    ImGui::SameLine();
  }
  ImGui::Text("%s", label);
  ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::CalcTextSize(value).x -
                  30.0f);
  ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "%s", value);

  ImDrawList *drawList = ImGui::GetWindowDrawList();
  ImVec2 pMin = ImGui::GetCursorScreenPos();
  drawList->AddLine(
      ImVec2(pMin.x, pMin.y + 4.0f),
      ImVec2(pMin.x + ImGui::GetContentRegionAvail().x, pMin.y + 4.0f),
      IM_COL32(255, 255, 255, 12));

  ImGui::Spacing();
  ImGui::Spacing();
}

void ProfileApp::drawStatProgressBar(const char *label, float percentage,
                                     const char *icon, ImVec4 iconColor) {
  ImGui::Spacing();
  if (icon) {
    ImGui::TextColored(iconColor, "%s", icon);
    ImGui::SameLine();
  }
  ImGui::Text("%s", label);
  ImGui::SameLine(ImGui::GetWindowWidth() - 60.0f);
  ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%.1f%%", percentage);

  ImGui::PushStyleColor(ImGuiCol_PlotHistogram,
                        ImVec4(0.0f, 0.478f, 1.0f, 1.0f)); // iOS Blue
  ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.25f, 0.25f, 0.28f, 0.4f));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

  ImGui::ProgressBar(percentage / 100.0f,
                     ImVec2(ImGui::GetContentRegionAvail().x, 6.0f), "");

  ImGui::PopStyleVar();
  ImGui::PopStyleColor(2);
  ImGui::Spacing();
}

void ProfileApp::drawJogadorMenu() {
  Inject<IStatsProvider> statsProvider;
  PlayerStats stats = statsProvider->GetPlayerStats();

  ImGui::BeginChild("##jogador_scroll", ImGui::GetContentRegionAvail(), false,
                    ImGuiWindowFlags_None);

  ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
  ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.18f, 0.18f, 0.20f, 0.8f));
  ImGui::BeginChild("##jogador_card",
                    ImVec2(ImGui::GetContentRegionAvail().x, 360), true,
                    ImGuiWindowFlags_NoScrollbar);

  // Dinheiro
  char moneyBuf[32];
  sprintf_s(moneyBuf, sizeof(moneyBuf), "$%d", stats.money);
  drawStatRow("Dinheiro Atual", moneyBuf, ICON_FA_MONEY_BILL_WAVE,
              ImVec4(0.196f, 0.843f, 0.294f, 1.0f));

  // Vida
  float healthPercent = (stats.health / stats.maxHealth) * 100.0f;
  healthPercent = std::clamp(healthPercent, 0.0f, 100.0f);
  drawStatProgressBar("Vida", healthPercent, ICON_FA_HEART,
                      ImVec4(1.0f, 0.231f, 0.188f, 1.0f));

  // Colete
  drawStatProgressBar("Colete", stats.armor, ICON_FA_SHIELD_ALT,
                      ImVec4(0.0f, 0.478f, 1.0f, 1.0f));

  // Gordura
  drawStatProgressBar("Gordura", stats.fat, ICON_FA_PIZZA_SLICE,
                      ImVec4(0.95f, 0.76f, 0.13f, 1.0f));

  // Musculo
  drawStatProgressBar("Musculo", stats.muscle, ICON_FA_DUMBBELL,
                      ImVec4(0.6f, 0.6f, 0.65f, 1.0f));

  drawStatProgressBar("Vigor", stats.stamina, ICON_FA_RUNNING,
      ImVec4(0.35f, 0.75f, 0.95f, 1.0f));

  drawStatProgressBar("Folego (Pulmao)", stats.lungCapacity, ICON_FA_LUNGS,
                      ImVec4(0.35f, 0.75f, 0.95f, 1.0f));

  drawStatProgressBar("Seducao", stats.sexAppeal, ICON_FA_HEART,
      ImVec4(1.0f, 0.4f, 0.7f, 1.0f));

  // Tempo de jogo
  int hours = stats.totalPlayTimeSeconds / 3600;
  int minutes = (stats.totalPlayTimeSeconds % 3600) / 60;
  int seconds = stats.totalPlayTimeSeconds % 60;
  char timeBuf[32];
  sprintf_s(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", hours, minutes,
            seconds);
  drawStatRow("Tempo de Jogo", timeBuf, ICON_FA_CLOCK,
              ImVec4(0.686f, 0.322f, 0.871f, 1.0f));

  // Refeicoes
  char mealsBuf[16];
  sprintf_s(mealsBuf, sizeof(mealsBuf), "%d", stats.mealsEaten);
  drawStatRow("Refeicoes Consumidas", mealsBuf, ICON_FA_UTENSILS,
              ImVec4(0.95f, 0.60f, 0.10f, 1.0f));

  ImGui::EndChild();
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();

  ImGui::EndChild();
}

void ProfileApp::drawHabilidadesMenu() {
  Inject<IStatsProvider> statsProvider;
  SkillStats stats = statsProvider->GetSkillStats();

  ImGui::BeginChild("##habilidades_scroll", ImGui::GetContentRegionAvail(),
                    false, ImGuiWindowFlags_None);

  ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
  ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.18f, 0.18f, 0.20f, 0.8f));

  // Group 1: Veiculos
  ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "VEICULOS");
  ImGui::BeginChild("##veiculos_card",
                    ImVec2(ImGui::GetContentRegionAvail().x, 190.0f), true,
                    ImGuiWindowFlags_NoScrollbar);
  drawStatProgressBar("Carro", stats.drivingSkill, ICON_FA_CAR,
                      ImVec4(0.0f, 0.478f, 1.0f, 1.0f));
  drawStatProgressBar("Moto", stats.bikeSkill, ICON_FA_MOTORCYCLE,
                      ImVec4(0.196f, 0.843f, 0.294f, 1.0f));
  drawStatProgressBar("Bicicleta", stats.cyclingSkill, ICON_FA_BICYCLE,
                      ImVec4(1.0f, 0.584f, 0.0f, 1.0f));
  drawStatProgressBar("Aviao / Helice", stats.flyingSkill, ICON_FA_PLANE,
                      ImVec4(0.686f, 0.322f, 0.871f, 1.0f));
  ImGui::EndChild();

  ImGui::Spacing();

  // Group 2: Armas
  ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "ARMAS");
  ImGui::BeginChild("##armas_card",
                    ImVec2(ImGui::GetContentRegionAvail().x, 490.0f), true,
                    ImGuiWindowFlags_NoScrollbar);
  drawStatProgressBar("Pistola (9mm)", stats.pistolSkill, ICON_FA_CROSSHAIRS,
                      ImVec4(1.0f, 0.231f, 0.188f, 1.0f));
  drawStatProgressBar("Pistola Silenciada", stats.silencedPistolSkill, ICON_FA_CROSSHAIRS,
                      ImVec4(1.0f, 0.231f, 0.188f, 1.0f));
  drawStatProgressBar("Desert Eagle", stats.desertEagleSkill, ICON_FA_CROSSHAIRS,
                      ImVec4(1.0f, 0.231f, 0.188f, 1.0f));
  drawStatProgressBar("Escopeta", stats.shotgunSkill, ICON_FA_CROSSHAIRS,
                      ImVec4(1.0f, 0.231f, 0.188f, 1.0f));
  drawStatProgressBar("Escopeta Cano Curto", stats.sawnoffShotgunSkill, ICON_FA_CROSSHAIRS,
                      ImVec4(1.0f, 0.231f, 0.188f, 1.0f));
  drawStatProgressBar("Escopeta de Combate", stats.combatShotgunSkill, ICON_FA_CROSSHAIRS,
                      ImVec4(1.0f, 0.231f, 0.188f, 1.0f));
  drawStatProgressBar("Submetralhadora Leve", stats.machinePistolSkill, ICON_FA_CROSSHAIRS,
                      ImVec4(1.0f, 0.231f, 0.188f, 1.0f));
  drawStatProgressBar("Submetralhadora (SMG)", stats.smgSkill,
                      ICON_FA_CROSSHAIRS, ImVec4(1.0f, 0.231f, 0.188f, 1.0f));
  drawStatProgressBar("AK-47", stats.ak47Skill, ICON_FA_CROSSHAIRS,
                      ImVec4(1.0f, 0.231f, 0.188f, 1.0f));
  drawStatProgressBar("M4", stats.m4Skill, ICON_FA_CROSSHAIRS,
                      ImVec4(1.0f, 0.231f, 0.188f, 1.0f));
  drawStatProgressBar("Rifle / Sniper", stats.sniperSkill, ICON_FA_CROSSHAIRS,
                      ImVec4(1.0f, 0.231f, 0.188f, 1.0f));
  ImGui::EndChild();

  ImGui::PopStyleColor();
  ImGui::PopStyleVar();

  ImGui::EndChild();
}

void ProfileApp::drawGangMenu() {
  Inject<IStatsProvider> statsProvider;
  GangStats stats = statsProvider->GetGangStats();

  ImGui::BeginChild("##gang_scroll", ImGui::GetContentRegionAvail(), false,
                    ImGuiWindowFlags_None);

  ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
  ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.18f, 0.18f, 0.20f, 0.8f));
  ImGui::BeginChild("##gang_card",
                    ImVec2(ImGui::GetContentRegionAvail().x, 330.0f), true,
                    ImGuiWindowFlags_NoScrollbar);

  // Respeito
  drawStatProgressBar("Respeito Total", stats.respect, ICON_FA_STAR,
                      ImVec4(1.0f, 0.85f, 0.1f, 1.0f));

  // Territorio %
  drawStatProgressBar("Territorio GSF", stats.territoryControlledPercentage,
                      ICON_FA_MAP_MARKED_ALT,
                      ImVec4(0.196f, 0.843f, 0.294f, 1.0f));

  // Strongest Gang (1st)
  drawStatRow("1ª Maior Gangue", stats.strongestGangName.c_str(), ICON_FA_CROWN,
              ImVec4(1.0f, 0.843f, 0.0f, 1.0f)); // Gold

  // 2nd Strongest Gang
  drawStatRow("2ª Maior Gangue", stats.secondStrongestGangName.c_str(), ICON_FA_CROWN,
              ImVec4(0.75f, 0.75f, 0.75f, 1.0f)); // Silver

  // 3rd Strongest Gang
  drawStatRow("3ª Maior Gangue", stats.thirdStrongestGangName.c_str(), ICON_FA_CROWN,
              ImVec4(0.80f, 0.50f, 0.20f, 1.0f)); // Bronze

  // Territories Held
  char terrBuf[16];
  sprintf_s(terrBuf, sizeof(terrBuf), "%d", stats.territoriesHeld);
  drawStatRow("Territorios Controlados", terrBuf, ICON_FA_MAP_PIN,
              ImVec4(0.196f, 0.843f, 0.294f, 1.0f));

  // Members Recruited
  char recBuf[16];
  sprintf_s(recBuf, sizeof(recBuf), "%d", stats.recruitedMembersCount);
  drawStatRow("Membros Recrutados", recBuf, ICON_FA_USERS,
              ImVec4(0.0f, 0.478f, 1.0f, 1.0f));

  // Max recruits
  char maxRecBuf[16];
  sprintf_s(maxRecBuf, sizeof(maxRecBuf), "%d", stats.maxRecruitsCount);
  drawStatRow("Capacidade Max Recrutas", maxRecBuf, ICON_FA_USER_PLUS,
              ImVec4(0.686f, 0.322f, 0.871f, 1.0f));

  ImGui::EndChild();
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();

  ImGui::EndChild();
}

void ProfileApp::drawCrimesMenu() {
  Inject<IStatsProvider> statsProvider;
  CrimeStats stats = statsProvider->GetCrimeStats();

  ImGui::BeginChild("##crimes_scroll", ImGui::GetContentRegionAvail(), false,
                    ImGuiWindowFlags_None);

  ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
  ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.18f, 0.18f, 0.20f, 0.8f));
  ImGui::BeginChild("##crimes_card",
                    ImVec2(ImGui::GetContentRegionAvail().x, 260.0f), true,
                    ImGuiWindowFlags_NoScrollbar);

  char killedBuf[16];
  sprintf_s(killedBuf, sizeof(killedBuf), "%d", stats.peopleKilled);
  drawStatRow("Pessoas Mortas", killedBuf, ICON_FA_SKULL,
              ImVec4(1.0f, 0.231f, 0.188f, 1.0f));

  char stolenBuf[16];
  sprintf_s(stolenBuf, sizeof(stolenBuf), "%d", stats.carsStolen);
  drawStatRow("Carros Roubados", stolenBuf, ICON_FA_CAR_CRASH,
              ImVec4(1.0f, 0.584f, 0.0f, 1.0f));

  char destBuf[16];
  sprintf_s(destBuf, sizeof(destBuf), "%d", stats.vehiclesDestroyed);
  drawStatRow("Veiculos Destruidos", destBuf, ICON_FA_FIRE,
              ImVec4(0.95f, 0.40f, 0.10f, 1.0f));

  char bustedBuf[16];
  sprintf_s(bustedBuf, sizeof(bustedBuf), "%d", stats.bustedCount);
  drawStatRow("Prisoes Sofridas", bustedBuf, ICON_FA_LOCK,
              ImVec4(0.6f, 0.6f, 0.65f, 1.0f));

  char wastedBuf[16];
  sprintf_s(wastedBuf, sizeof(wastedBuf), "%d", stats.wastedCount);
  drawStatRow("Mortes do Jogador", wastedBuf, ICON_FA_HOSPITAL,
              ImVec4(0.0f, 0.478f, 1.0f, 1.0f));

  char evadeBuf[16];
  sprintf_s(evadeBuf, sizeof(evadeBuf), "%d", stats.starsEvaded);
  drawStatRow("Fugas da Policia", evadeBuf, ICON_FA_RUNNING,
              ImVec4(0.196f, 0.843f, 0.294f, 1.0f));

  ImGui::EndChild();
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();

  ImGui::EndChild();
}

void ProfileApp::drawProgressoMenu() {
  Inject<IStatsProvider> statsProvider;
  ProgressStats stats = statsProvider->GetProgressStats();

  ImGui::BeginChild("##progresso_scroll", ImGui::GetContentRegionAvail(), false,
                    ImGuiWindowFlags_None);

  ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
  ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.18f, 0.18f, 0.20f, 0.8f));
  ImGui::BeginChild("##progresso_card",
                    ImVec2(ImGui::GetContentRegionAvail().x, 390.0f), true,
                    ImGuiWindowFlags_NoScrollbar);

  // Conclusao
  drawStatProgressBar("Conclusao Geral", stats.completionPercentage,
                      ICON_FA_CHART_PIE, ImVec4(0.0f, 0.478f, 1.0f, 1.0f));

  // Missions Story
  char missBuf[32];
  sprintf_s(missBuf, sizeof(missBuf), "%d / %d", stats.storyMissionsCompleted,
            stats.storyMissionsTotal);
  drawStatRow("Missoes da Historia", missBuf, ICON_FA_CHECK_CIRCLE,
              ImVec4(0.196f, 0.843f, 0.294f, 1.0f));

  // Missions Failed
  char failBuf[16];
  sprintf_s(failBuf, sizeof(failBuf), "%d", stats.missionsFailed);
  drawStatRow("Missoes Falhas", failBuf, ICON_FA_TIMES_CIRCLE,
              ImVec4(1.0f, 0.231f, 0.188f, 1.0f));

  // Cheats
  char cheatsBuf[32];
  if (stats.cheatsUsedCount > 0) {
    sprintf_s(cheatsBuf, sizeof(cheatsBuf), "Sim (%d)", stats.cheatsUsedCount);
  } else {
    sprintf_s(cheatsBuf, sizeof(cheatsBuf), "Nao");
  }
  drawStatRow("Cheats Utilizados", cheatsBuf, ICON_FA_EXCLAMATION_TRIANGLE,
              ImVec4(1.0f, 0.584f, 0.0f, 1.0f));

  // Photos
  char photosBuf[16];
  sprintf_s(photosBuf, sizeof(photosBuf), "%d", stats.photosTaken);
  drawStatRow("Fotos Tiradas", photosBuf, ICON_FA_CAMERA,
              ImVec4(0.6f, 0.6f, 0.65f, 1.0f));

  // Collectibles
  char oystersBuf[32];
  sprintf_s(oystersBuf, sizeof(oystersBuf), "%d / %d", stats.oystersCollected,
            stats.oystersTotal);
  drawStatRow("Ostras", oystersBuf, ICON_FA_FISH,
              ImVec4(0.35f, 0.75f, 0.95f, 1.0f));

  char horseshoesBuf[32];
  sprintf_s(horseshoesBuf, sizeof(horseshoesBuf), "%d / %d",
            stats.horseshoesCollected, stats.horseshoesTotal);
  drawStatRow("Ferros de Cavalo", horseshoesBuf, ICON_FA_HORSE,
              ImVec4(0.95f, 0.60f, 0.10f, 1.0f));

  char tagsBuf[32];
  sprintf_s(tagsBuf, sizeof(tagsBuf), "%d / %d", stats.tagsSprayed,
            stats.tagsTotal);
  drawStatRow("Tags Pixadas", tagsBuf, ICON_FA_SPRAY_CAN,
              ImVec4(0.196f, 0.843f, 0.294f, 1.0f));

  char snapshotsBuf[32];
  sprintf_s(snapshotsBuf, sizeof(snapshotsBuf), "%d / %d", stats.snapshotsTaken,
            stats.snapshotsTotal);
  drawStatRow("Snapshots", snapshotsBuf, ICON_FA_IMAGE,
              ImVec4(0.686f, 0.322f, 0.871f, 1.0f));

  // Indicator highlight for 100% completion
  if (stats.completionPercentage >= 100.0f) {
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Button,
                          ImVec4(0.196f, 0.843f, 0.294f, 1.0f));
    ImGui::Button("CONQUISTA 100% ATINGIDA!",
                  ImVec2(ImGui::GetContentRegionAvail().x, 30.0f));
    ImGui::PopStyleColor();
  }

  ImGui::EndChild();
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();

  ImGui::EndChild();
}
