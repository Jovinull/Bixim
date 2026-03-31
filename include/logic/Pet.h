// =============================================================================
// Bixim — Pet: FSM and Status Data
// File   : include/logic/Pet.h
// =============================================================================
// The Pet class encapsulates all virtual pet state: vital statistics,
// the finite state machine, and the tick-driven decay/transition logic.
//
// Lifecycle:
//   HATCHING  → egg animation; transitions to IDLE after HATCHING_DURATION ticks
//   IDLE      → default state; stats decay over time
//   EATING    → triggered by feed(); transitions back to IDLE after EATING_DURATION
//   SLEEPING  → triggered by sleep(); energy regenerates; wake() exits
//   SICK      → hunger >= SICK_HUNGER_THRESHOLD for SICK_TRIGGER_TICKS ticks
//   DEAD      → sick for DEAD_TRIGGER_TICKS ticks; terminal state
//
// Stat decay rates (at LOGIC_HZ = 10 ticks/s):
//   Hunger    : +1 every HUNGER_DECAY_TICKS    (180 ticks = 18 s)
//   Happiness : -1 every HAPPINESS_DECAY_TICKS (300 ticks = 30 s)
//   Energy    : -1 every ENERGY_DECAY_TICKS    (240 ticks = 24 s) — not while sleeping
//   Hygiene   : -1 every HYGIENE_DECAY_TICKS   (360 ticks = 36 s)
//
// All stats are clamped to [0, 100].
// =============================================================================
#pragma once

#include <cstdint>

// =============================================================================
// Pet States
// =============================================================================
enum class PetState : uint8_t {
    HATCHING  = 0,
    IDLE      = 1,
    EATING    = 2,
    SLEEPING  = 3,
    SICK      = 4,
    DEAD      = 5,
};

// =============================================================================
// Pet Statistics
// =============================================================================
struct PetStats {
    uint8_t  hunger;     // 0 = full, 100 = starving
    uint8_t  happiness;  // 0 = miserable, 100 = ecstatic
    uint8_t  energy;     // 0 = exhausted, 100 = fully rested
    uint8_t  hygiene;    // 0 = filthy, 100 = spotless
    uint32_t age_ticks;  // total logic ticks lived
};

// =============================================================================
// Decay / Transition Constants
// =============================================================================
static constexpr uint32_t HATCHING_DURATION      = 50;   // ticks in hatching
static constexpr uint32_t EATING_DURATION        = 30;   // ticks in eating animation
static constexpr uint32_t HUNGER_DECAY_TICKS     = 180;  // ticks per +1 hunger
static constexpr uint32_t ENERGY_DECAY_TICKS     = 240;  // ticks per -1 energy
static constexpr uint32_t HAPPINESS_DECAY_TICKS  = 300;  // ticks per -1 happiness
static constexpr uint32_t HYGIENE_DECAY_TICKS    = 360;  // ticks per -1 hygiene
static constexpr uint8_t  SICK_HUNGER_THRESHOLD  = 80;   // hunger value that triggers sick countdown
static constexpr uint32_t SICK_TRIGGER_TICKS     = 300;  // ticks at high hunger before SICK
static constexpr uint32_t DEAD_TRIGGER_TICKS     = 600;  // ticks in SICK before DEAD
static constexpr uint8_t  FEED_HUNGER_RESTORE    = 30;   // hunger reduced per feed
static constexpr uint8_t  PLAY_HAPPINESS_RESTORE = 20;   // happiness gained per play
static constexpr uint8_t  SLEEP_ENERGY_RESTORE   = 1;    // energy per tick while sleeping
static constexpr uint8_t  CLEAN_HYGIENE_RESTORE  = 40;   // hygiene restored per clean

// =============================================================================
// Pet Class
// =============================================================================
class Pet {
public:
    Pet();

    // --- Tick ---
    // Called once per logic tick. Advances age, decays stats, drives FSM.
    void tick();

    // --- Actions ---
    // Each returns true if the action was accepted in the current state,
    // false if the pet is in a state that cannot accept it (e.g., DEAD).
    bool feed();
    bool play();
    bool sleep();
    bool wake();
    bool clean();

    // --- Accessors ---
    PetState       getState() const { return m_state; }
    const PetStats& getStats() const { return m_stats; }

private:
    PetStats m_stats;
    PetState m_state;

    // Counters for decay accumulation
    uint32_t m_hunger_counter;
    uint32_t m_energy_counter;
    uint32_t m_happiness_counter;
    uint32_t m_hygiene_counter;

    // Counters for state timers
    uint32_t m_state_ticks;   // ticks spent in current state
    uint32_t m_sick_ticks;    // ticks hunger has been above threshold (for SICK trigger)
    uint32_t m_dead_ticks;    // ticks spent in SICK state (for DEAD trigger)

    // Internal helpers
    void decayStats();
    void advanceFSM();
    void enterState(PetState s);
};
