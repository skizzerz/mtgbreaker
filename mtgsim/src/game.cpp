#include "game.hpp"
#include "action.hpp"
#include "stackable.hpp"
#include "player.hpp"
#include "playerlogic.hpp"
#include <algorithm>
#include <iostream>
#include <cassert>

void Game::play() {
    Player* loser = nullptr;
    try {
        do
        {
            untap();
            upkeep();
            if (turn_number != 0)
                draw();
            premain();
            combegin();
            attack();
            block();
            damage();
            comend();
            postmain();
            endstep();
            cleanup();
            // flip the active player
            active_player = active_player == p1 ? p2 : p1;
            turn_number += 1;
        } while (1);
    } catch (Player* p) {
        loser = p;
    }

    std::cerr << "Player " << (size_t)loser << " loses." << std::endl;
}

void Game::untap() {
    for (auto c : battlefield)
    {
        if (c->owner == active_player)
            c->tapped = false;
    }
}

void Game::upkeep() {
}

void Game::draw() {
    active_player->draw();
}

void Game::premain() {
    priority = active_player;
    passed_players = 0;
    state = PRECOMBAT_MAIN;

    resolve_priority();
}
void Game::resolve_priority() {
    do {
        while (passed_players != 2)
        {
            if (priority->loss_pending)
            {
                throw priority;
            }

            auto p = priority;
            Action* a = p->ai->next_action(this, p);
            if (a == nullptr)
            {
                // passed
                ++passed_players;
                priority = priority == p1 ? p2 : p1;
                continue;
            }
            auto check_res = a->check(this, p);
            if (check_res != nullptr)
            {
                std::cerr << "Someone messed up: " << check_res << std::endl;
                throw std::runtime_error(check_res);
            }
            a->enact(this, p);
            passed_players = 0;
        }

        if (stack.size() > 0)
        {
            auto s = stack.back();
            stack.pop_back();
            s->resolve(this);
            delete s;
        }
    } while (stack.size() > 0);
}

void Game::combegin() {}
void Game::attack() {}
void Game::block() {}
void Game::damage() {}
void Game::comend() {}
void Game::postmain() {}
void Game::endstep() {}
void Game::cleanup() {
    auto p = active_player;
    int diff = p->hand.size() - 7;
    if (diff > 0)
    {
        std::cerr << "P" << (size_t)p << " is forced to discard." << std::endl;
        // Give player a chance to choose
        p->ai->discard(this, p, diff);
        assert(p->hand.size() >= (size_t)diff);
        p->hand.erase(p->hand.end() - diff, p->hand.end());
    }
}
