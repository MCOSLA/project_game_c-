#pragma once
#include <string>
#include <vector>

#include "types.h"

GameState InitGame(const std::string& data_path);

Floor BuildFloor(FloorLevel level, const std::vector<Recipe>& all_recipes,
                 const std::vector<Customer>& all_customers,
                 const GameConfig& config);

bool AddToInventory(Player& player, IngredientType item);

void RemoveFromInventory(Player& player, int slot_index);

bool HasIngredients(const Player& player, const Recipe& recipe);

void ConsumeIngredients(Player& player, const Recipe& recipe);

bool CanProgressToNextFloor(const Floor& floor);

bool CheckLossCondition(const GameState& state);

void CheckAndAwardUpgrade(GameState& state);

void CheckStoryTriggers(GameState& state, Floor& floor, int customer_index);

Ending DetermineEnding(const GameState& state);
