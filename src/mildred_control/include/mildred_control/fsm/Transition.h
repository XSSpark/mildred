#pragma once

#include <memory>
#include <type_traits>

#include "Event.h"
#include "Machine.h"

namespace Mildred {
    template<class S, class E, typename = std::enable_if<std::is_base_of<Event, E>::value>>
    class BaseTransition {
      public:
        virtual std::shared_ptr<S> transit(const E &event) = 0;
    };

    template<
        class S,
        class E,
        class FS,
        class TS,
        typename = std::enable_if<std::is_base_of<Event, E>::value>,
        typename = std::enable_if<std::is_base_of<std::shared_ptr<S>, FS>::value>,
        typename = std::enable_if<std::is_base_of<std::shared_ptr<S>, TS>::value>>
    class Transition : public BaseTransition<S, E> {
      public:
        Transition(std::shared_ptr<FS> fromState, std::shared_ptr<TS> toState) :
            fromState(fromState), toState(toState) {};
        ~Transition() = default;

        std::shared_ptr<S> transit(const E &event) override {
            std::cout << "Attempting transition from state " << fromState->name() << " to state " << toState->name() << std::endl;

            if (std::static_pointer_cast<S>(fromState) == std::static_pointer_cast<S>(toState)) {
                std::cout << " - States are the same, no transition possible";
                return nullptr;
            }

            if (!fromState->onExit(event)) {
                std::cout << "  - State " << fromState->name() << " rejected transition on exit" << std::endl;
                return fromState;
            }

            if (!toState->onEnter(event)) {
                std::cout << "  - State " << toState->name() << " rejected transition on enter" << std::endl;
                fromState->onEnter(event);
                return fromState;
            }

            return toState;
        }

      private:
        std::shared_ptr<FS> fromState{nullptr};
        std::shared_ptr<TS> toState{nullptr};
    };
}