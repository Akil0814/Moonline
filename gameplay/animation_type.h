#pragma once

#include <array>
#include <string_view>

enum class AnimationType
{
    Idle,
    IdleIntoLong,
    IdleLong,
    Strengthen,

    WalkEnter,
    WalkLoop,
    RunEnter,
    RunLoop,
    RunExit,

    DashBack,
    DashForward,
    AirDashBack,
    AirDashForward,

    JumpStart,
    JumpUp,
    JumpApex,
    JumpFall,
    JumpLand,

    AttackNormal,
    AttackAir,
    AttackRangedGround,
    AttackRangedAir,
    AttackSpecial,

    GuardGround,
    GuardAir,
    Hurt,
    Knockdown,
    KnockdownAirEnter,
    KnockdownAirLoop,
    KnockdownAirLand,
    Getup,
    Start
};

inline constexpr std::array<AnimationType, 32> all_animation_types{
    AnimationType::Idle,
    AnimationType::IdleIntoLong,
    AnimationType::IdleLong,
    AnimationType::Strengthen,
    AnimationType::WalkEnter,
    AnimationType::WalkLoop,
    AnimationType::RunEnter,
    AnimationType::RunLoop,
    AnimationType::RunExit,
    AnimationType::DashBack,
    AnimationType::DashForward,
    AnimationType::AirDashBack,
    AnimationType::AirDashForward,
    AnimationType::JumpStart,
    AnimationType::JumpUp,
    AnimationType::JumpApex,
    AnimationType::JumpFall,
    AnimationType::JumpLand,
    AnimationType::AttackNormal,
    AnimationType::AttackAir,
    AnimationType::AttackRangedGround,
    AnimationType::AttackRangedAir,
    AnimationType::AttackSpecial,
    AnimationType::GuardGround,
    AnimationType::GuardAir,
    AnimationType::Hurt,
    AnimationType::Knockdown,
    AnimationType::KnockdownAirEnter,
    AnimationType::KnockdownAirLoop,
    AnimationType::KnockdownAirLand,
    AnimationType::Getup,
    AnimationType::Start
};

inline constexpr std::string_view to_animation_path(AnimationType type)
{
    switch (type)
    {
    case AnimationType::Idle:
        return "base/idle";
    case AnimationType::IdleIntoLong:
        return "base/idle_into_long";
    case AnimationType::IdleLong:
        return "base/idle_long";
    case AnimationType::Strengthen:
        return "base/strengthen";
    case AnimationType::WalkEnter:
        return "movement/walk/enter";
    case AnimationType::WalkLoop:
        return "movement/walk/loop";
    case AnimationType::RunEnter:
        return "movement/run/enter";
    case AnimationType::RunLoop:
        return "movement/run/loop";
    case AnimationType::RunExit:
        return "movement/run/exit";
    case AnimationType::DashBack:
        return "movement/dash/back";
    case AnimationType::DashForward:
        return "movement/dash/front";
    case AnimationType::AirDashBack:
        return "movement/dash/in_air_back";
    case AnimationType::AirDashForward:
        return "movement/dash/in_air_front";
    case AnimationType::JumpStart:
        return "movement/jump/start";
    case AnimationType::JumpUp:
        return "movement/jump/rise";
    case AnimationType::JumpApex:
        return "movement/jump/apex";
    case AnimationType::JumpFall:
        return "movement/jump/fall";
    case AnimationType::JumpLand:
        return "movement/jump/land";
    case AnimationType::AttackNormal:
        return "attack/normal";
    case AnimationType::AttackAir:
        return "attack/aerial";
    case AnimationType::AttackRangedGround:
        return "attack/ranged/ground";
    case AnimationType::AttackRangedAir:
        return "attack/ranged/air";
    case AnimationType::AttackSpecial:
        return "attack/special";
    case AnimationType::GuardGround:
        return "reaction/guard/ground";
    case AnimationType::GuardAir:
        return "reaction/guard/air";
    case AnimationType::Hurt:
        return "reaction/hurt";
    case AnimationType::Knockdown:
        return "reaction/knockdown";
    case AnimationType::KnockdownAirEnter:
        return "reaction/knockdown_air/enter";
    case AnimationType::KnockdownAirLoop:
        return "reaction/knockdown_air/loop";
    case AnimationType::KnockdownAirLand:
        return "reaction/knockdown_air/land";
    case AnimationType::Getup:
        return "reaction/getup";
    case AnimationType::Start:
        return "reaction/start";
    }

    return {};
}
