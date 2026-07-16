#include "engine/gameplay_support/collision/actor_collision_rig.h"
#include "engine/gameplay_support/collision/gameplay_collision_events.h"
#include "engine/gameplay_support/collision/gameplay_collision_listener.h"
#include "engine/gameplay_support/collision/gameplay_collision_service.h"
#include "engine/gameplay_support/collision/team_relation_resolver.h"
#include "engine/physics/collider.h"
#include "engine/physics/collision_contact.h"
#include "engine/physics/collision_system.h"
#include "engine/physics/physics_body.h"
#include "engine/physics/physics_system.h"
#include "tests/support/test_assertions.h"

#include <iostream>
#include <type_traits>
#include <vector>

using moonline::tests::require;

namespace
{
struct EmptyEntry
{
};
}

int main()
{
    using namespace elysia::physics;
    using namespace elysia::gameplay::collision;

    static_assert(std::is_abstract_v<GameplayCollisionService>);
    static_assert(std::is_abstract_v<TeamRelationResolver>);

    Collider collider;
    require(collider.id == InvalidColliderId, "Collider IDs must default to invalid");
    require(collider.filter.category == 0, "Collider categories must default to empty");
    require(collider.filter.mask == 0xffffffffu, "Collider masks must accept all categories by default");
    require(collider.response == CollisionResponse::Block, "Colliders must default to blocking intent");
    require(collider.enabled, "Colliders must default to enabled");

    ActorCollisionRig rig;
    require(rig.owner == InvalidActorId, "Actor rigs must default to an invalid owner");
    require(rig.body == InvalidColliderId, "Actor body references must default to invalid");
    require(rig.push_box == InvalidColliderId, "Actor push-box references must default to invalid");
    require(teams::Player != teams::Enemy, "Player and enemy presets must be distinct");
    require(teams::Neutral != teams::Player, "Neutral and player presets must be distinct");

    PhysicsBody body;
    body.velocity = elysia::core::Vector2(12.0f, -4.0f);

    const std::vector<EmptyEntry> entries(1);
    PhysicsSystem physics_system;
    physics_system.step(entries, 1.0 / 60.0);
    require(body.velocity == elysia::core::Vector2(12.0f, -4.0f),
        "The physics scaffold must not mutate bodies");

    CollisionSystem collision_system;
    collision_system.dispatch_events(entries);

    GameplayCollisionListener listener;
    listener.on_body_contact(BodyContactEvent{});
    listener.on_push_box_overlap(PushBoxOverlapEvent{});
    listener.on_hit_overlap(HitOverlapEvent{});

    std::cout << "physics framework contract tests passed\n";
    return 0;
}
