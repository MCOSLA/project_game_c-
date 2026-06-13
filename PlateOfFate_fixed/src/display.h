#pragma once

#include <string>
#include <vector>

#include "types.h"

void ClearScreen();

void PrintSeparator();

void PrintTitle(const std::string& title);

void PressEnter();

void ShowMainMenu();

void ShowFloorBanner(FloorLevel floor);

void ShowInventory(const Player& player, const std::vector<Recipe>& recipes);

void ShowStorageMenu(const Floor& floor, const Player& player,
                     const std::vector<Recipe>& recipes);

void ShowKitchenMenu(const Floor& floor, const Player& player,
                     const std::vector<Recipe>& recipes);

void ShowCustomerList(const Floor& floor);

void ShowCustomerDialog(const Customer& customer);

void ShowRecipeChoice(const std::vector<Recipe>& recipes);

void ShowRecipeBook(const std::vector<Recipe>& recipes);

void ShowStoryEvent(const std::string& text);

void ShowEnding(Ending ending);

void ShowUpgradeMenu(std::vector<Recipe>& recipes);
