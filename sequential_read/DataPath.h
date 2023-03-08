#pragma once
#ifndef DATAPATH_H
#define DATAPATH_H

#include <filesystem>
#include <optional>
namespace fs = std::filesystem;

std::optional<fs::path> prepareDataPath();

#endif