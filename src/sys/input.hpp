#pragma once
#include <cstdint>

namespace ECS {

    struct InputState_t {
        bool left    { false };
        bool right   { false };
    bool up      { false };
    bool down    { false };
    bool space   { false };
    bool restart { false };
    bool attack  { false };
    };

    void initInput();
    InputState_t& getInputState();

}
