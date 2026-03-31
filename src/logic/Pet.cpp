// =============================================================================
// Bixim — Pet: FSM and Status Implementation
// File   : src/logic/Pet.cpp
// =============================================================================
#include "logic/Pet.h"

// =============================================================================
// Constructor
// =============================================================================
Pet::Pet()
    : m_state(PetState::HATCHING)
    , m_hunger_counter(0)
    , m_energy_counter(0)
    , m_happiness_counter(0)
    , m_hygiene_counter(0)
    , m_state_ticks(0)
    , m_sick_ticks(0)
    , m_dead_ticks(0)
{
    m_stats.hunger     = 20;
    m_stats.happiness  = 80;
    m_stats.energy     = 100;
    m_stats.hygiene    = 100;
    m_stats.age_ticks  = 0;
}

// =============================================================================
// tick
// =============================================================================
void Pet::tick()
{
    if (m_state == PetState::DEAD) return;

    m_stats.age_ticks++;
    m_state_ticks++;

    decayStats();
    advanceFSM();
}

// =============================================================================
// decayStats
// =============================================================================
void Pet::decayStats()
{
    // Hunger decays (increases) always, even while sleeping.
    m_hunger_counter++;
    if (m_hunger_counter >= HUNGER_DECAY_TICKS) {
        m_hunger_counter = 0;
        if (m_stats.hunger < 100) m_stats.hunger++;
    }

    // Energy decays only when awake.
    if (m_state != PetState::SLEEPING) {
        m_energy_counter++;
        if (m_energy_counter >= ENERGY_DECAY_TICKS) {
            m_energy_counter = 0;
            if (m_stats.energy > 0) m_stats.energy--;
        }
    } else {
        // While sleeping, energy restores each tick.
        if (m_stats.energy < 100) {
            m_stats.energy += SLEEP_ENERGY_RESTORE;
            if (m_stats.energy > 100) m_stats.energy = 100;
        }
    }

    // Happiness decays always.
    m_happiness_counter++;
    if (m_happiness_counter >= HAPPINESS_DECAY_TICKS) {
        m_happiness_counter = 0;
        if (m_stats.happiness > 0) m_stats.happiness--;
    }

    // Hygiene decays always.
    m_hygiene_counter++;
    if (m_hygiene_counter >= HYGIENE_DECAY_TICKS) {
        m_hygiene_counter = 0;
        if (m_stats.hygiene > 0) m_stats.hygiene--;
    }
}

// =============================================================================
// advanceFSM
// =============================================================================
void Pet::advanceFSM()
{
    switch (m_state) {

    case PetState::HATCHING:
        if (m_state_ticks >= HATCHING_DURATION) {
            enterState(PetState::IDLE);
        }
        break;

    case PetState::IDLE:
        // Sick trigger: hunger has been above threshold for SICK_TRIGGER_TICKS.
        if (m_stats.hunger >= SICK_HUNGER_THRESHOLD) {
            m_sick_ticks++;
            if (m_sick_ticks >= SICK_TRIGGER_TICKS) {
                enterState(PetState::SICK);
            }
        } else {
            m_sick_ticks = 0;
        }
        break;

    case PetState::EATING:
        if (m_state_ticks >= EATING_DURATION) {
            enterState(PetState::IDLE);
        }
        break;

    case PetState::SLEEPING:
        // Sleeping exits only via wake(). No auto-transition here.
        break;

    case PetState::SICK:
        // Still reset sick_ticks if hunger drops below threshold while sick.
        if (m_stats.hunger >= SICK_HUNGER_THRESHOLD) {
            m_dead_ticks++;
            if (m_dead_ticks >= DEAD_TRIGGER_TICKS) {
                enterState(PetState::DEAD);
            }
        } else {
            // Feeding can bring pet back from SICK.
            m_dead_ticks = 0;
            enterState(PetState::IDLE);
        }
        break;

    case PetState::DEAD:
        // Terminal — handled by early return in tick().
        break;
    }
}

// =============================================================================
// enterState
// =============================================================================
void Pet::enterState(PetState s)
{
    m_state       = s;
    m_state_ticks = 0;
}

// =============================================================================
// Actions
// =============================================================================

bool Pet::feed()
{
    if (m_state == PetState::DEAD || m_state == PetState::HATCHING) return false;

    // Reduce hunger.
    if (m_stats.hunger > FEED_HUNGER_RESTORE) {
        m_stats.hunger -= FEED_HUNGER_RESTORE;
    } else {
        m_stats.hunger = 0;
    }

    // If sleeping, wake the pet to eat.
    if (m_state == PetState::SLEEPING) {
        enterState(PetState::EATING);
    } else {
        enterState(PetState::EATING);
    }
    return true;
}

bool Pet::play()
{
    if (m_state == PetState::DEAD     ||
        m_state == PetState::HATCHING ||
        m_state == PetState::SLEEPING) return false;

    if (m_stats.happiness <= 100 - PLAY_HAPPINESS_RESTORE) {
        m_stats.happiness += PLAY_HAPPINESS_RESTORE;
    } else {
        m_stats.happiness = 100;
    }
    return true;
}

bool Pet::sleep()
{
    if (m_state != PetState::IDLE) return false;
    enterState(PetState::SLEEPING);
    return true;
}

bool Pet::wake()
{
    if (m_state != PetState::SLEEPING) return false;
    enterState(PetState::IDLE);
    return true;
}

bool Pet::clean()
{
    if (m_state == PetState::DEAD || m_state == PetState::HATCHING) return false;

    if (m_stats.hygiene <= 100 - CLEAN_HYGIENE_RESTORE) {
        m_stats.hygiene += CLEAN_HYGIENE_RESTORE;
    } else {
        m_stats.hygiene = 100;
    }
    return true;
}
