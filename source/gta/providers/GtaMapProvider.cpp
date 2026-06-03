#include "GtaMapProvider.h"
#include "../../core/LocalizationManager.h"
#include <RenderWare.h>
#include <cstdio>
#include <game_sa/CMenuManager.h>
#include <game_sa/CPlayerPed.h>
#include <game_sa/CRadar.h>
#include <game_sa/CStreaming.h>
#include <game_sa/CText.h>
#include <game_sa/CTheZones.h>
#include <game_sa/CTxdStore.h>
#include <game_sa/common.h>

extern int *gRadarTxdIds; // int gRadarTextures[12][12]

IDirect3DTexture9 *GtaMapProvider::GetD3DTexture(RwRaster *raster) {
  if (!raster)
    return nullptr;
  // The D3D9 texture pointer is stored immediately following the base RwRaster
  // structure.
  return *reinterpret_cast<IDirect3DTexture9 **>(
      reinterpret_cast<char *>(raster) + sizeof(RwRaster));
}

bool GtaMapProvider::GetPlayerPosition(float &x, float &y, float &heading) {
  CPlayerPed *player = FindPlayerPed(-1);
  if (!player)
    return false;

  CVector pos = player->GetPosition();
  x = pos.x;
  y = pos.y;

  // Get orientation using GetHeading() which returns radians (0 is South,
  // increases CCW)
  float rad = player->GetHeading();
  float headingDeg = -rad * (180.0f / 3.14159265f);

  // Normalize to [0, 360)
  while (headingDeg < 0.0f)
    headingDeg += 360.0f;
  while (headingDeg >= 360.0f)
    headingDeg -= 360.0f;

  heading = headingDeg;
  return true;
}

bool GtaMapProvider::GetWaypointPosition(float &x, float &y) {
  int blipHandle = FrontEndMenuManager.m_nTargetBlipIndex;
  if (blipHandle <= 0)
    return false;

  int index = CRadar::GetActualBlipArrayIndex(blipHandle);
  if (index < 0 || index >= 175)
    return false;

  tRadarTrace &trace = CRadar::ms_RadarTrace[index];
  if (!trace.m_bInUse)
    return false;

  x = trace.m_vecPos.x;
  y = trace.m_vecPos.y;
  return true;
}

void GtaMapProvider::SetWaypointPosition(float x, float y) {
  ClearWaypoint();

  // Create new target blip
  // BLIP_COORD = 4, BLIP_COLOUR_DESTINATION = 8, BLIP_DISPLAY_BOTH = 3
  int handle =
      CRadar::SetCoordBlip(BLIP_COORD, CVector(x, y, 0.0f),
                           BLIP_COLOUR_DESTINATION, BLIP_DISPLAY_BOTH, nullptr);
  if (handle > 0) {
    CRadar::SetBlipSprite(handle, RADAR_SPRITE_WAYPOINT); // 41
    FrontEndMenuManager.m_nTargetBlipIndex = handle;
  }
}

void GtaMapProvider::ClearWaypoint() {
  int handle = FrontEndMenuManager.m_nTargetBlipIndex;
  if (handle > 0) {
    CRadar::ClearBlip(handle);
    FrontEndMenuManager.m_nTargetBlipIndex = 0;
  }
}

void GtaMapProvider::LoadMapTiles() {
  for (int y = 0; y < 12; ++y) {
    for (int x = 0; x < 12; ++x) {
      CRadar::RequestMapSection(x, y);
    }
  }
  CStreaming::LoadAllRequestedModels(false);
}

ImTextureID GtaMapProvider::GetTileTexture(int index) {
  if (index < 0 || index >= 144)
    return 0;

  int txdSlot = gRadarTxdIds[index];
  if (txdSlot < 0)
    return 0;

  TxdDef *txd = CTxdStore::ms_pTxdPool->GetAt(txdSlot);
  if (!txd)
    return 0;

  // Load dynamically on demand if not loaded
  if (!txd->m_pRwDictionary) {
    int tx = index % 12;
    int ty = index / 12;
    CRadar::RequestMapSection(tx, ty);
    CStreaming::LoadAllRequestedModels(false);
    txd = CTxdStore::ms_pTxdPool->GetAt(txdSlot);
    if (!txd || !txd->m_pRwDictionary)
      return 0;
  }

  char name[32];
  sprintf(name, "radar%02d", index);

  RwTexture *rwTex =
      RwTexDictionaryFindNamedTexture(txd->m_pRwDictionary, name);
  if (!rwTex || !rwTex->raster)
    return 0;

  IDirect3DTexture9 *d3dTex = GetD3DTexture(rwTex->raster);
  return reinterpret_cast<ImTextureID>(d3dTex);
}

ImTextureID GtaMapProvider::GetBlipTexture(int spriteId) {
  if (spriteId < 0 || spriteId >= 64)
    return 0;

  CSprite2d &sprite = CRadar::RadarBlipSprites[spriteId];
  if (!sprite.m_pTexture || !sprite.m_pTexture->raster)
    return 0;

  IDirect3DTexture9 *d3dTex = GetD3DTexture(sprite.m_pTexture->raster);
  return reinterpret_cast<ImTextureID>(d3dTex);
}

std::vector<MapBlip> GtaMapProvider::GetActiveBlips() {
  std::vector<MapBlip> active;

  for (int i = 0; i < 175; ++i) { // MAX_RADAR_TRACES = 175
    tRadarTrace &trace = CRadar::ms_RadarTrace[i];
    if (!trace.m_bInUse)
      continue;

    int sprite = trace.m_nRadarSprite;

    // Filter relevant blip sprites (commercial, utility, and social
    // establishments)
    bool relevant =
        (sprite == 5) ||  // Airport (RADAR_SPRITE_AIRYARD)
        (sprite == 6) ||  // Ammu-Nation (RADAR_SPRITE_AMMUGUN)
        (sprite == 7) ||  // Barber (RADAR_SPRITE_BARBERS)
        (sprite == 9) ||  // Boatyard (RADAR_SPRITE_BOATYARD)
        (sprite == 10) || // Burger Shot (RADAR_SPRITE_BURGERSHOT)
        (sprite == 14) || // Cluckin' Bell (RADAR_SPRITE_CHICKEN)
        (sprite == 17) || // Diner (RADAR_SPRITE_DINER)
        (sprite == 21) || // Girlfriend (RADAR_SPRITE_GIRLFRIEND)
        (sprite == 22) || // Hospital (RADAR_SPRITE_HOSTPITAL)
        (sprite == 25) || // Caligula's Casino (RADAR_SPRITE_MAFIACASINO)
        (sprite == 27) || // Mod Garage (RADAR_SPRITE_MODGARAGE)
        (sprite == 29) || // Pizza (RADAR_SPRITE_PIZZA)
        (sprite == 30) || // Police (RADAR_SPRITE_POLICE)
        (sprite == 31) || // Property Purchased (RADAR_SPRITE_PROPERTYG)
        (sprite == 32) || // Property For Sale (RADAR_SPRITE_PROPERTYR)
        (sprite == 33) || // Race (RADAR_SPRITE_RACE)
        (sprite == 35) || // Safehouse (RADAR_SPRITE_SAVEGAME)
        (sprite == 36) || // Driving School (RADAR_SPRITE_SCHOOL)
        (sprite == 39) || // Tattoo (RADAR_SPRITE_TATTOO)
        (sprite == 44) || // Four Dragons Casino (RADAR_SPRITE_TRIADSCASINO)
        (sprite == 45) || // Clothes (RADAR_SPRITE_TSHIRT)
        (sprite == 47) || // Zero's RC Shop (RADAR_SPRITE_ZERO)
        (sprite == 48) || // Dance Club (RADAR_SPRITE_DATEDISCO)
        (sprite == 49) || // Bar (RADAR_SPRITE_DATEDRINK)
        (sprite == 50) || // Date Restaurant (RADAR_SPRITE_DATEFOOD)
        (sprite == 54) || // Gym (RADAR_SPRITE_GYM)
        (sprite == 55) || // Impound Lot (RADAR_SPRITE_IMPOUND)
        (sprite == 63);   // Pay 'N' Spray (RADAR_SPRITE_SPRAY)

    if (!relevant)
      continue;

    // Check if blip has been revealed
    bool revealed = CRadar::HasThisBlipBeenRevealed(i);
    if (!revealed)
      continue;

    // Skip blips that are hidden/undiscovered (m_nBlipDisplay == 0, i.e.
    // BLIP_DISPLAY_NEITHER)
    if (trace.m_nBlipDisplay == 0)
      continue;

    // Resolve generic name based on sprite
    std::string baseName = TR("maps.place.default");
    switch (sprite) {
    case 5:
      baseName = TR("maps.place.airport");
      break;
    case 6:
      baseName = TR("maps.place.ammu");
      break;
    case 7:
      baseName = TR("maps.place.barber");
      break;
    case 9:
      baseName = TR("maps.place.boat_dock");
      break;
    case 10:
      baseName = TR("maps.place.burger_shot");
      break;
    case 14:
      baseName = TR("maps.place.cluckin_bell");
      break;
    case 17:
      baseName = TR("maps.place.restaurant");
      break;
    case 21:
      baseName = TR("maps.place.girlfriend");
      break;
    case 22:
      baseName = TR("maps.place.hospital");
      break;
    case 25:
      baseName = TR("maps.place.caligulas");
      break;
    case 27:
      baseName = TR("maps.place.mod_garage");
      break;
    case 29:
      baseName = TR("maps.place.pizza");
      break;
    case 30:
      baseName = TR("maps.place.police");
      break;
    case 31:
      baseName = TR("maps.place.property_owned");
      break;
    case 32:
      baseName = TR("maps.place.property_sale");
      break;
    case 33:
      baseName = TR("maps.place.race");
      break;
    case 35:
      baseName = TR("maps.place.safehouse");
      break;
    case 36:
      baseName = TR("maps.place.flight_school");
      break;
    case 39:
      baseName = TR("maps.place.tattoo");
      break;
    case 44:
      baseName = TR("maps.place.four_dragons");
      break;
    case 45:
      baseName = TR("maps.place.clothes");
      break;
    case 47:
      baseName = TR("maps.place.zero");
      break;
    case 48:
      baseName = TR("maps.place.club");
      break;
    case 49:
      baseName = TR("maps.place.bar");
      break;
    case 50:
      baseName = TR("maps.place.date_restaurant");
      break;
    case 54:
      baseName = TR("maps.place.gym");
      break;
    case 55:
      baseName = TR("maps.place.impound");
      break;
    case 63:
      baseName = TR("maps.place.pay_n_spray");
      break;
    }

    // Resolve zone name (e.g. "Ganton")
    std::string zoneName = "";
    CZone *zone = CTheZones::FindSmallestZoneForPosition(trace.m_vecPos, false);
    if (zone) {
      const char *translatedZone = TheText.Get(zone->m_szLabel);
      if (translatedZone && strlen(translatedZone) > 0) {
        zoneName = translatedZone;
      }
    }

    std::string fullName = baseName;
    if (!zoneName.empty()) {
      fullName += " (" + zoneName + ")";
    }

    active.push_back({trace.m_vecPos.x, trace.m_vecPos.y, sprite,
                      static_cast<int>(trace.m_nColour), fullName, i});
  }

  return active;
}
