#include "GtaCameraProvider.h"
#include <game_sa/common.h>
#include <game_sa/CPlayerPed.h>
#include <game_sa/CPad.h>
#include <game_sa/enums/eScriptCommands.h>
#include <extensions/ScriptCommands.h>
#include <game_sa/CStreaming.h>
#include <game_sa/CWeaponInfo.h>
#include <cmath>

void GtaCameraProvider::SetActive(bool active) {
    CPlayerPed* player = FindPlayerPed(-1);
    if (!player) return;

    if (active) {
        m_active = true;
        m_selfie = false;
        m_selfieZoom = 1.6f;
        m_selfieAngleX = 0.0f;
        m_selfieAngleY = 0.0f;
        
        // Request and load camera weapon model (367)
        if (!CStreaming::HasModelLoaded(367)) {
            CStreaming::RequestModel(367, 2);
            CStreaming::LoadAllRequestedModels(false);
        }

        // Save player's currently held weapon
        m_originalWeapon = player->m_aWeapons[player->m_nSelectedWepSlot].m_eWeaponType;
        
        // Check if player has camera (slot 9) or needs ammo
        int cameraSlot = 9;

        if (player->m_aWeapons[cameraSlot].m_eWeaponType != WEAPONTYPE_CAMERA || player->m_aWeapons[cameraSlot].m_nAmmoTotal <= 0) {
            player->GiveWeapon(WEAPONTYPE_CAMERA, 100, true);
            m_gaveCamera = true;
        } else {
            m_gaveCamera = false;
        }
        
        // Force equip the camera
        player->SetCurrentWeapon(WEAPONTYPE_CAMERA);

        // Hide game HUD and radar
        plugin::Command<COMMAND_DISPLAY_HUD>(false);
        plugin::Command<COMMAND_DISPLAY_RADAR>(false);
    } 
    else {
        m_active = false;
        m_selfie = false;
        
        // Restore player control of camera
        plugin::Command<COMMAND_RESTORE_CAMERA_JUMPCUT>();
        
        // Restore original weapon first (switch away from camera)
        if (m_originalWeapon != -1 && m_originalWeapon != WEAPONTYPE_CAMERA) {
            player->SetCurrentWeapon(eWeaponType(m_originalWeapon));
        }
        m_originalWeapon = -1;

        // If we gave the camera, strip it properly using script command (async safe)
        if (m_gaveCamera) {
            plugin::Command<COMMAND_REMOVE_WEAPON_FROM_CHAR>(player, WEAPONTYPE_CAMERA);
            CStreaming::SetModelIsDeletable(367);
            m_gaveCamera = false;
        }

        // Restore game HUD and radar
        plugin::Command<COMMAND_DISPLAY_HUD>(true);
        plugin::Command<COMMAND_DISPLAY_RADAR>(true);
    }
}



void GtaCameraProvider::SetSelfieMode(bool selfie) {
    m_selfie = selfie;
    m_selfieAngleX = 0.0f;
    m_selfieAngleY = 0.0f;
    if (!selfie) {
        // Clear fixed camera and restore control so it aims natively
        plugin::Command<COMMAND_RESTORE_CAMERA_JUMPCUT>();
    }
}

bool GtaCameraProvider::TakePhoto() {
    CPlayerPed* player = FindPlayerPed(-1);
    if (!player) return false;

    m_captureFrames = 3;
    return true;
}

void GtaCameraProvider::Update(float dt) {
    if (!m_active) return;

    CPlayerPed* player = FindPlayerPed(-1);
    if (!player || !player->IsAlive()) {
        SetActive(false);
        return;
    }

    CPad* pad = CPad::GetPad(0);
    if (!pad) return;

    // Force camera weapon to be active (prevent weapon scrolls)
    if (player->m_aWeapons[player->m_nSelectedWepSlot].m_eWeaponType != WEAPONTYPE_CAMERA) {
        player->SetCurrentWeapon(WEAPONTYPE_CAMERA);
    }

    // Disable player actions while taking photos (jump, sprint, enter vehicle, crouch, weapon changing)
    pad->NewState.ButtonSquare = 0;   // Jump / reverse
    pad->NewState.ButtonCross = 0;    // Sprint / accelerate
    pad->NewState.ButtonTriangle = 0; // Enter vehicle / exit
    pad->NewState.ShockButtonL = 0;   // Crouch
    pad->NewState.ShockButtonR = 0;   // Look behind
    pad->NewState.Select = 0;         // Prevent Tab key from showing stats

    bool takingSelfiePhoto = false;
    if (m_captureFrames > 0) {
        m_captureFrames--;
        takingSelfiePhoto = true;
    }

    if (m_selfie) {
        // --- SELFIE MODE ---
        // Read mouse movement to rotate camera around player
        float mouseX = pad->NewMouseControllerState.x;
        float mouseY = pad->NewMouseControllerState.y;

        // Zero out mouse inputs so the game doesn't process them for anything else
        pad->NewMouseControllerState.x = 0.0f;
        pad->NewMouseControllerState.y = 0.0f;

        m_selfieAngleX += mouseX * 0.005f;
        m_selfieAngleY -= mouseY * 0.005f; // invert Y for intuitive vertical look

        // Clamp angles
        if (m_selfieAngleY < -0.6f) m_selfieAngleY = -0.6f;
        if (m_selfieAngleY > 0.8f) m_selfieAngleY = 0.8f;

        CVector playerPos = player->GetPosition();
        float yaw = player->GetHeading() + m_selfieAngleX;
        float pitch = m_selfieAngleY;

        float cosPitch = cos(pitch);
        float sinPitch = sin(pitch);
        
        float dx = -sin(yaw) * cosPitch;
        float dy = cos(yaw) * cosPitch;
        float dz = 0.8f + sinPitch * m_selfieZoom;
        
        // Position camera relative to CJ using yaw/pitch offset and selfie zoom distance
        CVector camPos = playerPos + CVector(dx * m_selfieZoom, dy * m_selfieZoom, dz);
        
        // Lock camera position
        plugin::Command<COMMAND_SET_FIXED_CAMERA_POSITION>(camPos.x, camPos.y, camPos.z, 0.0f, 0.0f, 0.0f);
        
        // Point camera at CJ's head/chest area (mode 15 = focus tracking, style 2 = jump cut)
        plugin::Command<COMMAND_POINT_CAMERA_AT_CHAR>(player, 15, 2);

        if (takingSelfiePhoto) {
            // Force Aim + Fire keys to trigger camera weapon shot
            pad->NewState.LeftShoulder1 = 255;
            pad->NewState.ButtonCircle = 255;
            pad->NewMouseControllerState.rmb = true;
            pad->NewMouseControllerState.lmb = true;
        }
    } 
    else {
        // --- NORMAL FIRST PERSON MODE ---
        // Force the player to hold aim so they look through the viewfinder
        pad->NewState.LeftShoulder1 = 255;
        pad->NewMouseControllerState.rmb = true;

        if (takingSelfiePhoto) {
            // Force Fire key
            pad->NewState.ButtonCircle = 255;
            pad->NewMouseControllerState.lmb = true;
        }
    }
}

void GtaCameraProvider::AdjustZoom(float delta) {
    if (m_selfie) {
        m_selfieZoom -= delta * 0.2f;
        if (m_selfieZoom < 1.0f) m_selfieZoom = 1.0f;
        if (m_selfieZoom > 3.0f) m_selfieZoom = 3.0f;
    }
}
