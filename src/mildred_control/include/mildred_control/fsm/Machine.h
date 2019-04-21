#pragma once

#include <any>
#include <iostream>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <vector>

#include "Event.h"
#include "State.h"
#include "Transition.h"

namespace Mildred {
    class Machine {
        public:
            Machine();
            ~Machine() = default;
            void addState(std::shared_ptr<State> state, bool initial = false);

            template<
                class E,
                class FS,
                class TS,
                typename = std::enable_if<std::is_base_of<Event, E>::value>,
                typename = std::enable_if<std::is_base_of<std::shared_ptr<State>, FS>::value>,
                typename = std::enable_if<std::is_base_of<std::shared_ptr<State>, TS>::value>>
            void addTransition(std::shared_ptr<FS> fromState, std::shared_ptr<TS> toState) {
                std::cout << "Adding transition from state " << fromState->name() << " to state " << toState->name() << " for event " << typeid(E).name() << " (" << typeid(E).hash_code() << ")" << std::endl;
                transitions[fromState].emplace(typeid(E), std::static_pointer_cast<BaseTransition<E>>(std::make_shared<Transition<E, FS, TS>>(fromState, toState)));
            }

            template<class E, typename = std::enable_if<std::is_base_of<Event, E>::value>>
            void handleEvent(const E &event) {
                if (currentState == nullptr) {
                    std::cerr << "No current state" << std::endl;
                    return;
                }

                auto possibleTransitions = transitions.find(currentState);
                if (possibleTransitions == transitions.end()) {
                    std::cerr << "No possible transitions from state " << currentState->name() << std::endl;
                    return;
                }

                auto transition = possibleTransitions->second.find(typeid(E));
                if (transition == possibleTransitions->second.end()) {
                    std::cerr << "No transition from state " << currentState->name() << " for event " << typeid(E).name() << " (" << typeid(E).hash_code() << ")" << std::endl;
                    return;
                }

                std::any_cast<std::shared_ptr<BaseTransition<E>>>(transition->second)->transit(event);
            }

        protected:
            std::vector<std::shared_ptr<State>>                                                       states;
            std::shared_ptr<State>                                                                    currentState;
            std::unordered_map<std::shared_ptr<State>, std::unordered_map<std::type_index, std::any>> transitions;
    };
}