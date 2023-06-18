// Created by block on 6/18/23.

#pragma once

#include <xenomods/Utils.hpp>

#include "xenomods/engine/fw/Document.hpp"
#include "xenomods/engine/mm/MathTypes.hpp"
#include "xenomods/engine/mm/mtl/FixedVector.hpp"

#if XENOMODS_CODENAME(bfsw)
namespace game {

	enum class PcID {
		None,
		Shulk,
		Reyn,
		FioraHoms,
		Dunban,
		Sharla,
		Riki,
		Melia,
		FioraMachina,
		Dickson,
		Mumkhar,
		Alvis,
		DunbanPrelude,
		DunbanCopy,
		Kino,
		Nene
	};

	enum class BdatId {
		// lol
	};

	class DataCharaStatus {
	   public:
		int level;
		int levelMax; // sorta read-only
		int hp;
		int unk1;
		float always100;
		unsigned int unk2;
		unsigned int unk3;
		unsigned int bonusExp;
		unsigned int unk4;
		unsigned int unk5;
		float unk6;
		std::uint8_t tp_atk;
		std::uint8_t tp_arts;
		std::uint8_t tp_atkdmg;
		std::uint8_t tp_artsdmg;
		INSERT_PADDING_BYTES(0x10);
		int baseHp;
		unsigned short baseStrength;
		unsigned short baseAgility;
		unsigned short baseEther;
		INSERT_PADDING_BYTES(0xA0-2);
		int hpMax;
		INSERT_PADDING_BYTES(0x4C);
	};

	class DataCharaValiableStatus {
	   public:
		static void setupMapEnemy(game::DataCharaStatus& status, const fw::Document& doc, BdatId pcid, unsigned short param_4, int param_5);
		static void setupPc(game::DataCharaStatus& status, const fw::Document& doc, PcID pcid, unsigned short param_4);
	};

	class DataPc {
	   public:
		PcID pcid;
		DataCharaStatus status;
		// a lot more
	};

	class DataParty {
	   public:
		DataPc* getMember(PcID pcid);
	};

}
#endif

template<>
struct fmt::formatter<game::PcID> : fmt::formatter<std::string_view> {
	template<typename FormatContext>
	inline auto format(game::PcID pcid, FormatContext& ctx) {
		std::string_view name;

		// clang-format off
		switch(pcid) {
			using enum game::PcID;

			case Shulk: name = "Shulk"; break;
			case Reyn: name = "Reyn"; break;
			case FioraHoms: name = "Fiora (Homs)"; break;
			case Dunban: name = "Dunban"; break;
			case Sharla: name = "Sharla"; break;
			case Riki: name = "Riki"; break;
			case Melia: name = "Melia"; break;
			case FioraMachina: name = "Fiora (Machina)"; break;
			case Dickson: name = "Dickson"; break;
			case Mumkhar: name = "Mumkhar"; break;
			case Alvis: name = "Alvis"; break;
			case DunbanPrelude: name = "Dunban (Prelude)"; break;
			case DunbanCopy: name = "Dunban Copy?"; break;
			case Kino: name = "Kino"; break;
			case Nene: name = "Nene"; break;
			default: name = "Unknown - " + std::to_string(reinterpret_cast<std::underlying_type_t<game::PcID>&>(pcid)); break;
		}
		// clang-format on

		return formatter<std::string_view>::format(name, ctx);
	}
};