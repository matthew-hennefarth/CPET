// Copyright(c) 2020-Present, Matthew R. Hennefarth
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace cpet::constants {
constexpr int PDB_XCOORD_START = 31;
constexpr int PDB_YCOORD_START = 39;
constexpr int PDB_ZCOORD_START = 47;
constexpr int PDB_CHARGE_START = 55;
constexpr int PDB_COORD_WIDTH = 8;
constexpr int PDB_CHARGE_WIDTH = 8;
constexpr int PDB_MIN_LINE_LENGTH = 26;
constexpr int PDB_CHAIN_START = 21;
constexpr int PDB_CHAIN_WIDTH = 2;
constexpr int PDB_RESNUM_START = 22;
constexpr int PDB_RESNUM_WIDTH = 4;
constexpr int PDB_ATOMID_START = 12;
constexpr int PDB_ATOMID_WIDTH = 4;

constexpr int PQR_XCOORD_INDEX = 6;
constexpr int PQR_YCOORD_INDEX = 7;
constexpr int PQR_ZCOORD_INDEX = 8;
constexpr int PQR_CHARGE_INDEX = 9;
constexpr int PQR_CHAIN_INDEX = 4;
constexpr int PQR_RESNUM_INDEX = 5;
constexpr int PQR_ATOMID_INDEX = 2;
constexpr int PQR_MIN_INDEX = PQR_RESNUM_INDEX;

enum class FileType { pdb, pqr };

}  // namespace cpet::constants
#endif  // CONSTANTS_H
