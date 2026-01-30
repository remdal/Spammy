//
//  RMDLFPS.hpp
//  Spammy
//
//  Created by Rémy on 29/01/2026.
//

#ifndef RMDLFPS_hpp
#define RMDLFPS_hpp

#include <simd/simd.h>
#include <vector>
#include <string>
#include <functional>
#include <cmath>
#include <random>
#include <algorithm>

enum class WeaponType : uint8_t {
    Pistol,
    Rifle,
    Shotgun,
    Sniper,
    SMG,
    RocketLauncher,
    Count
};

enum class FireMode : uint8_t {
    Semi,       // Un tir par clic
    Auto,       // Tir continu
    Burst       // Rafale (3 tirs)
};

enum class AmmoType : uint8_t {
    Pistol,
    Rifle,
    Shell,
    Rocket,
    Count
};

enum class DamageType : uint8_t {
    Bullet,
    Explosion,
    Fall,
    Environment
};

struct WeaponStats
{
    std::string name;
    WeaponType type;
    FireMode fireMode;
    AmmoType ammoType;
    
    float damage           = 25.0f;
    float headshotMult     = 2.0f;
    float falloffStart     = 20.0f;    // Distance où les dégâts commencent à diminuer
    float falloffEnd       = 50.0f;    // Distance où dégâts = 50%
    
    float fireRate         = 10.0f;    // Tirs par seconde
    int   magSize          = 30;
    float reloadTime       = 2.0f;     // Secondes
    int   bulletsPerShot   = 1;        // >1 pour shotgun
    
    // Précision (en radians)
    float baseSpread       = 0.01f;    // Spread au repos
    float moveSpread       = 0.03f;    // Spread en mouvement
    float maxSpread        = 0.08f;    // Spread max (tir continu)
    float spreadPerShot    = 0.005f;   // Ajout par tir
    float spreadRecovery   = 0.1f;     // Récupération par seconde
    
    // Recul (en radians)
    float recoilPitch      = 0.02f;    // Recul vertical
    float recoilYaw        = 0.005f;   // Recul horizontal (random)
    float recoilRecovery   = 5.0f;     // Vitesse de retour
    
    // Visée (ADS)
    float adsSpreadMult    = 0.3f;     // Multiplicateur spread en ADS
    float adsSpeedMult     = 0.5f;     // Vitesse de mouvement en ADS
    float adsZoomFOV       = 50.0f;    // FOV en ADS (degrés)
    float adsTransition    = 0.2f;     // Temps pour ADS (secondes)
    
    // Balistique (pour projectiles, pas hitscan)
    bool  isHitscan        = true;
    float projectileSpeed  = 100.0f;   // Unités/seconde si !isHitscan
    float projectileGravity = 9.81f;   // Gravité si !isHitscan
    float explosionRadius  = 0.0f;     // >0 pour explosifs
};

namespace WeaponPresets {
    inline WeaponStats Pistol() {
        WeaponStats w;
        w.name = "Pistol"; w.type = WeaponType::Pistol;
        w.fireMode = FireMode::Semi; w.ammoType = AmmoType::Pistol;
        w.damage = 25.0f; w.fireRate = 5.0f; w.magSize = 12;
        w.reloadTime = 1.5f; w.baseSpread = 0.008f;
        w.recoilPitch = 0.025f; w.recoilYaw = 0.01f;
        return w;
    }
    
    inline WeaponStats AssaultRifle() {
        WeaponStats w;
        w.name = "Assault Rifle"; w.type = WeaponType::Rifle;
        w.fireMode = FireMode::Auto; w.ammoType = AmmoType::Rifle;
        w.damage = 20.0f; w.fireRate = 10.0f; w.magSize = 30;
        w.reloadTime = 2.2f; w.baseSpread = 0.012f;
        w.spreadPerShot = 0.004f; w.maxSpread = 0.06f;
        w.recoilPitch = 0.018f; w.recoilYaw = 0.008f;
        return w;
    }
    
    inline WeaponStats Shotgun() {
        WeaponStats w;
        w.name = "Shotgun"; w.type = WeaponType::Shotgun;
        w.fireMode = FireMode::Semi; w.ammoType = AmmoType::Shell;
        w.damage = 12.0f; w.fireRate = 1.2f; w.magSize = 8;
        w.reloadTime = 0.5f; // Par cartouche
        w.bulletsPerShot = 8; w.baseSpread = 0.05f;
        w.recoilPitch = 0.06f; w.recoilYaw = 0.02f;
        w.falloffStart = 5.0f; w.falloffEnd = 15.0f;
        return w;
    }
    
    inline WeaponStats Sniper() {
        WeaponStats w;
        w.name = "Sniper"; w.type = WeaponType::Sniper;
        w.fireMode = FireMode::Semi; w.ammoType = AmmoType::Rifle;
        w.damage = 100.0f; w.headshotMult = 2.5f;
        w.fireRate = 0.8f; w.magSize = 5; w.reloadTime = 3.0f;
        w.baseSpread = 0.001f; w.moveSpread = 0.04f;
        w.recoilPitch = 0.08f; w.recoilYaw = 0.01f;
        w.adsZoomFOV = 20.0f; w.adsSpreadMult = 0.1f;
        w.falloffStart = 100.0f; w.falloffEnd = 200.0f;
        return w;
    }
    
    inline WeaponStats SMG() {
        WeaponStats w;
        w.name = "SMG"; w.type = WeaponType::SMG;
        w.fireMode = FireMode::Auto; w.ammoType = AmmoType::Pistol;
        w.damage = 15.0f; w.fireRate = 15.0f; w.magSize = 35;
        w.reloadTime = 1.8f; w.baseSpread = 0.02f;
        w.spreadPerShot = 0.003f; w.maxSpread = 0.07f;
        w.recoilPitch = 0.012f; w.recoilYaw = 0.01f;
        w.falloffStart = 10.0f; w.falloffEnd = 30.0f;
        return w;
    }
    
    inline WeaponStats RocketLauncher() {
        WeaponStats w;
        w.name = "Rocket Launcher"; w.type = WeaponType::RocketLauncher;
        w.fireMode = FireMode::Semi; w.ammoType = AmmoType::Rocket;
        w.damage = 120.0f; w.fireRate = 0.5f; w.magSize = 1;
        w.reloadTime = 2.5f; w.baseSpread = 0.005f;
        w.recoilPitch = 0.04f;
        w.isHitscan = false; w.projectileSpeed = 30.0f;
        w.projectileGravity = 2.0f; w.explosionRadius = 5.0f;
        return w;
    }
}

struct WeaponState {
    WeaponStats stats;
    
    int   currentAmmo      = 30;
    int   reserveAmmo      = 90;
    float fireCooldown     = 0.0f;     // Temps avant prochain tir
    float reloadTimer      = 0.0f;     // >0 = en cours de reload
    int   burstCount       = 0;        // Tirs restants dans la rafale
    
    // Spread/Recul dynamiques
    float currentSpread    = 0.0f;
    float recoilAccumPitch = 0.0f;     // Recul accumulé (à appliquer à la caméra)
    float recoilAccumYaw   = 0.0f;
    
    // ADS
    float adsProgress      = 0.0f;     // 0 = hip, 1 = full ADS
    bool  isAiming         = false;
    
    bool isReloading() const { return reloadTimer > 0.0f; }
    bool canFire() const { return fireCooldown <= 0.0f && !isReloading() && currentAmmo > 0; }
};

struct Projectile
{
    simd_float3 position;
    simd_float3 velocity;
    float damage;
    float explosionRadius;
    float lifetime;          // Temps restant avant destruction
    int   ownerID;           // Qui a tiré
    bool  active = true;
};

struct HitResult
{
    bool  hit            = false;
    simd_float3 point    = {0,0,0};
    simd_float3 normal   = {0,1,0};
    float distance       = 0.0f;
    int   entityID       = -1;        // -1 = monde
    bool  isHeadshot     = false;
    float damageDealt    = 0.0f;
};

struct HealthComponent
{
    float health          = 100.0f;
    float maxHealth       = 100.0f;
    float armor           = 0.0f;
    float maxArmor        = 100.0f;
    float armorAbsorption = 0.5f;     // % dégâts absorbés par l'armure
    
    bool  isDead          = false;
    float deathTime       = 0.0f;
    float lastDamageTime  = 0.0f;
    float regenDelay      = 5.0f;     // Secondes avant regen
    float regenRate       = 10.0f;    // HP/seconde
    
    // Applique des dégâts, retourne les dégâts effectifs
    float takeDamage(float amount, DamageType type, float gameTime) {
        if (isDead) return 0.0f;
        
        float effectiveDamage = amount;
        
        // L'armure absorbe une partie
        if (armor > 0.0f) {
            float absorbed = amount * armorAbsorption;
            absorbed = std::min(absorbed, armor);
            armor -= absorbed;
            effectiveDamage -= absorbed;
        }
        
        health -= effectiveDamage;
        lastDamageTime = gameTime;
        
        if (health <= 0.0f) {
            health = 0.0f;
            isDead = true;
            deathTime = gameTime;
        }
        
        return effectiveDamage;
    }
    
    void heal(float amount) {
        if (isDead) return;
        health = std::min(health + amount, maxHealth);
    }
    
    void addArmor(float amount) {
        armor = std::min(armor + amount, maxArmor);
    }
    
    void update(float deltaTime, float gameTime) {
        if (isDead) return;
        
        // Regen passive après délai
        if (gameTime - lastDamageTime > regenDelay && health < maxHealth) {
            health = std::min(health + regenRate * deltaTime, maxHealth);
        }
    }
};

// =============================================================================
// SECTION 8: INPUTS FPS
// =============================================================================

struct FPSInput {
    // Existants dans ton système
    simd_float3 moveDirection = {0,0,0};  // Normalisé
    bool isMoving    = false;
    bool isSprinting = false;
    bool isCrouching = false;
    
    // Nouveaux pour les armes
    bool firePressed   = false;   // Bouton enfoncé
    bool fireJustPressed = false; // Juste ce frame
    bool aimPressed    = false;   // ADS (clic droit)
    bool reloadPressed = false;
    bool switchWeapon  = false;
    int  weaponSlot    = -1;      // 1-9 pour sélection directe, -1 = pas de changement
    float scrollDelta  = 0.0f;    // Molette pour changer d'arme
};

// HUD (à envoyer à UI)

struct HUDData
{
    std::string weaponName;
    int   currentAmmo    = 0;
    int   reserveAmmo    = 0;
    int   magSize        = 0;
    bool  isReloading    = false;
    float reloadProgress = 0.0f;   // 0-1
    
    // Crosshair
    float crosshairSpread = 0.0f;  // En pixels (à calculer selon FOV/resolution)
    bool  onTarget        = false; // True si on vise un ennemi
    
    // Vie
    float health    = 100.0f;
    float maxHealth = 100.0f;
    float armor     = 0.0f;
    float maxArmor  = 100.0f;
    
    // Indicateurs
    bool  lowAmmo   = false;       // < 25% du chargeur
    bool  lowHealth = false;       // < 25% de vie
    simd_float3 lastDamageDir = {0,0,0}; // Pour indicateur de dégâts
};

class FPSSystem
{
public:
    // Callback pour raycast (ton système de collision)
    // Retourne true si hit, remplit hitResult
    using RaycastFunc = std::function<bool(simd_float3 origin, simd_float3 dir,
                                           float maxDist, HitResult& outHit)>;
    
    // Callback pour appliquer des dégâts à une entité
    using DamageFunc = std::function<void(int entityID, float damage,
                                          DamageType type, simd_float3 hitPoint)>;
    
    // -------------------------------------------------------------------------
    // INITIALISATION
    // -------------------------------------------------------------------------
    
    FPSSystem();
    
    void setRaycastCallback(RaycastFunc func) { m_raycastFunc = func; }
    void setDamageCallback(DamageFunc func) { m_damageFunc = func; }
    
    // Initialise l'inventaire avec des armes
    void setupDefaultLoadout() {
        addWeapon(WeaponPresets::Pistol(), 36);
        addWeapon(WeaponPresets::AssaultRifle(), 120);
        addWeapon(WeaponPresets::Shotgun(), 24);
        equipWeapon(0);
    }
    
    void addWeapon(const WeaponStats& stats, int reserveAmmo) {
        WeaponState ws;
        ws.stats = stats;
        ws.currentAmmo = stats.magSize;
        ws.reserveAmmo = reserveAmmo;
        m_weapons.push_back(ws);
    }
    
    void equipWeapon(int index) {
        if (index >= 0 && index < (int)m_weapons.size()) {
            m_currentWeaponIndex = index;
        }
    }
    
    // -------------------------------------------------------------------------
    // UPDATE PRINCIPAL (appeler chaque frame)
    // -------------------------------------------------------------------------
    
    void update(const FPSInput& input, simd_float3 cameraPos, simd_float3 cameraForward,
                float deltaTime, float gameTime)
    {
        m_cameraPos = cameraPos;
        m_cameraForward = cameraForward;
        m_gameTime = gameTime;
        
        // Update vie du joueur
        m_playerHealth.update(deltaTime, gameTime);
        
        // Changement d'arme
        handleWeaponSwitch(input);
        
        if (m_weapons.empty()) return;
        WeaponState& weapon = currentWeapon();
        
        // Update timers
        weapon.fireCooldown = std::max(0.0f, weapon.fireCooldown - deltaTime);
        
        // Reload
        if (weapon.isReloading()) {
            weapon.reloadTimer -= deltaTime;
            if (weapon.reloadTimer <= 0.0f) {
                finishReload(weapon);
            }
        } else if (input.reloadPressed && weapon.currentAmmo < weapon.stats.magSize) {
            startReload(weapon);
        }
        
        // ADS
        updateADS(weapon, input, deltaTime);
        
        // Spread recovery
        float spreadRecovery = weapon.stats.spreadRecovery * deltaTime;
        weapon.currentSpread = std::max(weapon.stats.baseSpread,
                                        weapon.currentSpread - spreadRecovery);
        
        // Recoil recovery
        float recoilRecovery = weapon.stats.recoilRecovery * deltaTime;
        weapon.recoilAccumPitch = std::max(0.0f, weapon.recoilAccumPitch - recoilRecovery);
        weapon.recoilAccumYaw *= (1.0f - recoilRecovery);
        
        // Tir
        handleFiring(weapon, input, deltaTime);
        
        // Update projectiles
        updateProjectiles(deltaTime);
        
        // Update HUD data
        updateHUDData(weapon, input);
    }
    
    WeaponState& currentWeapon() { return m_weapons[m_currentWeaponIndex]; }
    const WeaponState& currentWeapon() const { return m_weapons[m_currentWeaponIndex]; }
    
    HealthComponent& playerHealth() { return m_playerHealth; }
    const HealthComponent& playerHealth() const { return m_playerHealth; }
    
    const HUDData& getHUDData() const { return m_hudData; }
    
    // Recul à appliquer à ta caméra (pitch, yaw) - consomme le recul
    simd_float2 consumeRecoil() {
        if (m_weapons.empty()) return {0,0};
        WeaponState& w = currentWeapon();
        simd_float2 r = {w.recoilAccumPitch, w.recoilAccumYaw};
        // On ne reset pas ici, le recovery s'en charge progressivement
        return r;
    }
    
    // FOV actuel (pour ADS zoom)
    float getCurrentFOV(float baseFOV) const {
        if (m_weapons.empty()) return baseFOV;
        const WeaponState& w = m_weapons[m_currentWeaponIndex];
        float adsFOV = w.stats.adsZoomFOV * (M_PI / 180.0f);
        return simd::lerp(baseFOV, adsFOV, w.adsProgress);
    }
    
    // Multiplicateur vitesse mouvement (pour ADS)
    float getMoveSpeedMultiplier() const {
        if (m_weapons.empty()) return 1.0f;
        const WeaponState& w = m_weapons[m_currentWeaponIndex];
        return simd::lerp(1.0f, w.stats.adsSpeedMult, w.adsProgress);
    }
    
    const std::vector<Projectile>& getProjectiles() const { return m_projectiles; }
    const std::vector<HitResult>& getLastHits() const { return m_lastHits; }
    
private:
    
    void handleWeaponSwitch(const FPSInput& input)
    {
        if (m_weapons.empty()) return;
        
        int newIndex = m_currentWeaponIndex;
        
        // Slot direct (1-9)
        if (input.weaponSlot >= 1 && input.weaponSlot <= (int)m_weapons.size()) {
            newIndex = input.weaponSlot - 1;
        }
        // Molette
        else if (std::abs(input.scrollDelta) > 0.1f) {
            newIndex += (input.scrollDelta > 0) ? 1 : -1;
            newIndex = (newIndex + m_weapons.size()) % m_weapons.size();
        }
        
        if (newIndex != m_currentWeaponIndex) {
            m_currentWeaponIndex = newIndex;
            // Reset ADS quand on change d'arme
            currentWeapon().adsProgress = 0.0f;
            currentWeapon().isAiming = false;
        }
    }
    
    // -------------------------------------------------------------------------
    // ADS (Aim Down Sights)
    // -------------------------------------------------------------------------
    
    void updateADS(WeaponState& weapon, const FPSInput& input, float dt) {
        weapon.isAiming = input.aimPressed;
        
        float targetADS = weapon.isAiming ? 1.0f : 0.0f;
        float adsSpeed = 1.0f / weapon.stats.adsTransition;
        
        if (weapon.adsProgress < targetADS) {
            weapon.adsProgress = std::min(targetADS, weapon.adsProgress + adsSpeed * dt);
        } else {
            weapon.adsProgress = std::max(targetADS, weapon.adsProgress - adsSpeed * dt);
        }
    }
    
    // -------------------------------------------------------------------------
    // RECHARGEMENT
    // -------------------------------------------------------------------------
    
    void startReload(WeaponState& weapon) {
        if (weapon.reserveAmmo <= 0) return;
        weapon.reloadTimer = weapon.stats.reloadTime;
    }
    
    void finishReload(WeaponState& weapon) {
        int needed = weapon.stats.magSize - weapon.currentAmmo;
        int available = std::min(needed, weapon.reserveAmmo);
        weapon.currentAmmo += available;
        weapon.reserveAmmo -= available;
        weapon.reloadTimer = 0.0f;
    }
    
    // -------------------------------------------------------------------------
    // TIR
    // -------------------------------------------------------------------------
    
    void handleFiring(WeaponState& weapon, const FPSInput& input, float dt) {
        bool shouldFire = false;
        
        switch (weapon.stats.fireMode) {
            case FireMode::Semi:
                shouldFire = input.fireJustPressed;
                break;
            case FireMode::Auto:
                shouldFire = input.firePressed;
                break;
            case FireMode::Burst:
                if (input.fireJustPressed && weapon.burstCount == 0) {
                    weapon.burstCount = 3;
                }
                shouldFire = weapon.burstCount > 0;
                break;
        }
        
        if (shouldFire && weapon.canFire()) {
            fire(weapon, input);
        }
    }
    
    void fire(WeaponState& weapon, const FPSInput& input) {
        m_lastHits.clear();
        
        // Calculer le spread actuel
        float spread = weapon.currentSpread;
        if (input.isMoving) spread += weapon.stats.moveSpread;
        if (weapon.isAiming) spread *= weapon.stats.adsSpreadMult;
        spread = std::min(spread, weapon.stats.maxSpread);
        
        // Tirer plusieurs projectiles (shotgun)
        for (int i = 0; i < weapon.stats.bulletsPerShot; i++) {
            simd_float3 dir = applySpread(m_cameraForward, spread);
            
            if (weapon.stats.isHitscan) {
                fireHitscan(weapon, dir);
            } else {
                fireProjectile(weapon, dir);
            }
        }
        
        // Appliquer recul
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        weapon.recoilAccumPitch += weapon.stats.recoilPitch;
        weapon.recoilAccumYaw += weapon.stats.recoilYaw * dist(m_rng);
        
        // Augmenter le spread
        weapon.currentSpread += weapon.stats.spreadPerShot;
        
        // Consommer munition et cooldown
        weapon.currentAmmo--;
        weapon.fireCooldown = 1.0f / weapon.stats.fireRate;
        
        if (weapon.burstCount > 0) weapon.burstCount--;
    }
    
    simd_float3 applySpread(simd_float3 dir, float spread) {
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        
        // Rotation aléatoire dans un cône
        float angle = dist(m_rng) * M_PI * 2.0f;
        float radius = dist(m_rng) * spread;
        
        // Construire une base orthonormale
        simd_float3 up = std::abs(dir.y) < 0.99f ?
            simd_make_float3(0,1,0) : simd_make_float3(1,0,0);
        simd_float3 right = simd_normalize(simd_cross(up, dir));
        up = simd_cross(dir, right);
        
        // Appliquer le spread
        simd_float3 offset = right * std::cos(angle) * radius + up * std::sin(angle) * radius;
        return simd_normalize(dir + offset);
    }
    
    void fireHitscan(WeaponState& weapon, simd_float3 dir) {
        if (!m_raycastFunc) return;
        
        HitResult hit;
        if (m_raycastFunc(m_cameraPos, dir, 1000.0f, hit)) {
            // Calculer dégâts avec falloff
            float damage = weapon.stats.damage;
            if (hit.distance > weapon.stats.falloffStart) {
                float falloffRange = weapon.stats.falloffEnd - weapon.stats.falloffStart;
                float falloffT = (hit.distance - weapon.stats.falloffStart) / falloffRange;
                falloffT = std::clamp(falloffT, 0.0f, 1.0f);
                damage *= (1.0f - falloffT * 0.5f); // Min 50% damage
            }
            
            // Headshot
            if (hit.isHeadshot) {
                damage *= weapon.stats.headshotMult;
            }
            
            hit.damageDealt = damage;
            m_lastHits.push_back(hit);
            
            // Appliquer dégâts via callback
            if (m_damageFunc && hit.entityID >= 0) {
                m_damageFunc(hit.entityID, damage, DamageType::Bullet, hit.point);
            }
        }
    }
    
    void fireProjectile(WeaponState& weapon, simd_float3 dir) {
        Projectile p;
        p.position = m_cameraPos;
        p.velocity = dir * weapon.stats.projectileSpeed;
        p.damage = weapon.stats.damage;
        p.explosionRadius = weapon.stats.explosionRadius;
        p.lifetime = 10.0f;
        p.ownerID = 0; // Player
        p.active = true;
        m_projectiles.push_back(p);
    }
    
    // -------------------------------------------------------------------------
    // UPDATE PROJECTILES
    // -------------------------------------------------------------------------
    
    void updateProjectiles(float dt)
    {
        for (auto& p : m_projectiles)
        {
            if (!p.active) continue;
            
            // Physique basique
            p.velocity.y -= currentWeapon().stats.projectileGravity * dt;
            simd_float3 movement = p.velocity * dt;
            
            // Raycast pour collision
            HitResult hit;
            if (m_raycastFunc && m_raycastFunc(p.position, simd_normalize(movement),
                                                simd_length(movement), hit)) {
                // Impact!
                if (p.explosionRadius > 0.0f) {
                    applyExplosion(hit.point, p.damage, p.explosionRadius);
                } else if (m_damageFunc && hit.entityID >= 0) {
                    m_damageFunc(hit.entityID, p.damage, DamageType::Bullet, hit.point);
                }
                p.active = false;
            } else {
                p.position += movement;
            }
            
            p.lifetime -= dt;
            if (p.lifetime <= 0.0f) p.active = false;
        }
        
        // Nettoyer les projectiles inactifs
        m_projectiles.erase(
            std::remove_if(m_projectiles.begin(), m_projectiles.end(),
                          [](const Projectile& p) { return !p.active; }),
            m_projectiles.end());
    }
    
    void applyExplosion(simd_float3 center, float damage, float radius) {
        // TODO: Trouver toutes les entités dans le rayon et appliquer dégâts
        // avec falloff basé sur la distance
        // Pour l'instant, juste un placeholder
        if (m_damageFunc) {
            // Dégâts au joueur si proche
            float distToPlayer = simd_length(m_cameraPos - center);
            if (distToPlayer < radius) {
                float falloff = 1.0f - (distToPlayer / radius);
                m_playerHealth.takeDamage(damage * falloff, DamageType::Explosion, m_gameTime);
            }
        }
    }
    
    // -------------------------------------------------------------------------
    // UPDATE HUD DATA
    // -------------------------------------------------------------------------
    
    void updateHUDData(const WeaponState& weapon, const FPSInput& input)
    {
        m_hudData.weaponName = weapon.stats.name;
        m_hudData.currentAmmo = weapon.currentAmmo;
        m_hudData.reserveAmmo = weapon.reserveAmmo;
        m_hudData.magSize = weapon.stats.magSize;
        m_hudData.isReloading = weapon.isReloading();
        m_hudData.reloadProgress = weapon.isReloading() ?
            1.0f - (weapon.reloadTimer / weapon.stats.reloadTime) : 0.0f;
        
        // Spread pour crosshair (converti en estimation pixels - à ajuster)
        m_hudData.crosshairSpread = weapon.currentSpread * 500.0f;
        
        m_hudData.health = m_playerHealth.health;
        m_hudData.maxHealth = m_playerHealth.maxHealth;
        m_hudData.armor = m_playerHealth.armor;
        m_hudData.maxArmor = m_playerHealth.maxArmor;
        
        m_hudData.lowAmmo = weapon.currentAmmo < weapon.stats.magSize / 4;
        m_hudData.lowHealth = m_playerHealth.health < m_playerHealth.maxHealth * 0.25f;
    }
    
    std::vector<WeaponState> m_weapons;
    int m_currentWeaponIndex = 0;
    
    HealthComponent m_playerHealth;
    HUDData m_hudData;
    
    std::vector<Projectile> m_projectiles;
    std::vector<HitResult> m_lastHits;
    
    simd_float3 m_cameraPos = { 0, 0, 0 };
    simd_float3 m_cameraForward = { 0, 0, -1 };
    float       m_gameTime = 0.0f;
    
    RaycastFunc m_raycastFunc;
    DamageFunc m_damageFunc;
    
    std::mt19937 m_rng;
};

#endif /* RMDLFPS_hpp */
