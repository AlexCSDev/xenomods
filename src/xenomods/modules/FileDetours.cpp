//
// Created by block on 1/9/2023.
//

#include "FileDetours.hpp"

#include "xenomods/engine/ml/Filesystem.hpp"
#include "xenomods/engine/mm/mtl/FixStr.hpp"
#include "xenomods/stuff/utils/debug_util.hpp"
#include "xenomods/stuff/utils/util.hpp"

namespace {



	bool DumpToFilesystem(std::string_view path, const void* buffer, std::size_t length) {
		if(!xenomods::NnFile::Preallocate(path, length)) {
			xenomods::g_Logger->LogError("Couldn't create/preallocate dump file \"{}\"", path);
		}

		xenomods::NnFile file(path, nn::fs::OpenMode_Write);

		if(!file) {
			xenomods::g_Logger->LogError("Couldn't open dump file \"{}\"", path);
			return false;
		}

		file.Write(buffer, length);
		file.Flush();
		return true;
	}

	void LoadFromFilesystem(std::string_view jackPath, void* buffer, std::size_t length) {
		xenomods::NnFile file(jackPath, nn::fs::OpenMode_Read);

		// This isn't really fatal since the override file
		// doesn't particularly need to exist. It's whatever really
		if(!file)
			return;

		file.Read(buffer, file.Size());
		xenomods::g_Logger->LogDebug("Loaded dump file \"{}\"", jackPath);
	}

	void FileDetourImpl(ml::FileHandleTh* fileHandle, ml::FileReadResult& readResult) {
		auto filename = std::string(fileHandle->filename.buffer);
		xenomods::NnFile::CleanPath(filename);

		// dump data reads to sd card
		if(xenomods::GetState().config.dumpFileReads) {
			auto path = fmt::format(XENOMODS_CONFIG_PATH "/{}/dump/{}/{:08x}.bin", XENOMODS_CODENAME_STR, filename, reinterpret_cast<uint32_t>(fileHandle->readStartOffset));
			DumpToFilesystem(path, fileHandle->mMemBuffer, readResult.bytesRead);
		}

		// load from loose sd card files
		if(xenomods::GetState().config.enableFileOverrides) {
			auto path = fmt::format(XENOMODS_CONFIG_PATH "/{}/override/{}/{:08x}.bin", XENOMODS_CODENAME_STR, filename, reinterpret_cast<uint32_t>(fileHandle->readStartOffset));
			LoadFromFilesystem(path, fileHandle->mMemBuffer, readResult.bytesRead);
		}
	}

	struct ReadFileHook : skylaunch::hook::Trampoline<ReadFileHook> {
		static uint Hook(ml::FileHandleTh*& fh, nn::fs::FileHandle& nnHandle, int unk, long offset, void* buffer, unsigned int buffersize, ml::FileReadResult& readres) {
			// used by maps (exclusively?)

			auto fileRes = Orig(fh, nnHandle, unk, offset, buffer, buffersize, readres);
			//xenomods::g_Logger->LogDebug("DevFileUtilNx::readFile: \"{}\" (start offset 0x{:08x} size 0x{:08x}, was compressed? {})", fh->filename.buffer, fh->readStartOffset, fh->readSize, readres.bWasCompressed);

			if(fileRes && readres.bFullyLoaded == 1)
				FileDetourImpl(fh, readres);

			return fileRes;
		}
	};

	struct ReadFileSliceHook : skylaunch::hook::Trampoline<ReadFileSliceHook> {
		static uint Hook(ml::FileHandleTh* fh, nn::fs::FileHandle& nnHandle, long unk, long offset, void* buffer, unsigned int buffersize, ml::FileReadResult& readres) {
			// used by most other things

			auto fileRes = Orig(fh, nnHandle, unk, offset, buffer, buffersize, readres);
			//xenomods::g_Logger->LogDebug("DevFileUtilNx::readFileSlice: \"{}\" (start offset 0x{:08x} size 0x{:08x}, was compressed? {})", fh->filename.buffer, fh->readStartOffset, fh->readSize, readres.bWasCompressed);

			if(fileRes && readres.bFullyLoaded == 1)
				FileDetourImpl(fh, readres);

			return fileRes;
		}
	};

} // namespace

namespace xenomods {

	void FileDetours::Initialize() {
		UpdatableModule::Initialize();
		g_Logger->LogDebug("Setting up file detours...");

#if !XENOMODS_CODENAME(bf3)
		ReadFileHook::HookAt("_ZN2ml13DevFileUtilNx8readFileERPNS_12FileHandleThERN2nn2fs10FileHandleEilPvjRNS_14FileReadResultE");
		ReadFileSliceHook::HookAt("_ZN2ml13DevFileUtilNx13readFileSliceEPNS_12FileHandleThERN2nn2fs10FileHandleEllPvjRNS_14FileReadResultE");
#else
		// ml::DevFileUtilNx::readFile(Slice)
		if (version::RuntimeVersion() == version::SemVer::v2_0_0) {
			ReadFileHook::HookFromBase(0x71012562a4);
			ReadFileSliceHook::HookFromBase(0x7101254ab0);
		}
		else if (version::RuntimeVersion() == version::SemVer::v2_1_0) {
			ReadFileHook::HookFromBase(0x71012565d4);
			ReadFileSliceHook::HookFromBase(0x7101254de0);
		}
		else if (version::RuntimeVersion() == version::SemVer::v2_1_1) {
			ReadFileHook::HookFromBase(0x7101256614);
			ReadFileSliceHook::HookFromBase(0x7101254e20);
		}
		else if (version::RuntimeVersion() == version::SemVer::v2_2_0) {
			ReadFileHook::HookFromBase(0x7101257184);
			ReadFileSliceHook::HookFromBase(0x7101255990);
		}
#endif
	}

	XENOMODS_REGISTER_MODULE(FileDetours);

} // namespace xenomods
