#include "FTBProfileStrategy.h"
#include "OneSixFTBInstance.h"

#include <FileSystem.h>

#include <QDir>
#include <QUuid>
#include <QJsonDocument>
#include <QJsonArray>

FTBProfileStrategy::FTBProfileStrategy(OneSixFTBInstance* instance) : OneSixProfileStrategy(instance)
{
}

void FTBProfileStrategy::loadDefaultBuiltinPatches()
{
	// FIXME: this should be here, but it needs us to be able to deal with multiple libraries paths
	// OneSixProfileStrategy::loadDefaultBuiltinPatches();
	auto mcVersion = m_instance->intendedVersionId();
	auto nativeInstance = dynamic_cast<OneSixFTBInstance *>(m_instance);

	ProfilePatchPtr minecraftPatch;
	{
		std::shared_ptr< VersionFile > file;
		auto mcJson = m_instance->versionsPath().absoluteFilePath(mcVersion + "/" + mcVersion + ".json");
		// load up the base minecraft patch
		if(QFile::exists(mcJson))
		{
			file = ProfileUtils::parseJsonFile(QFileInfo(mcJson), false);
			for(auto addLib: file->libraries)
			{
				addLib->setHint("local");
				addLib->setStoragePrefix(nativeInstance->librariesPath().absolutePath());
			}
		}
		else
		{
			file = std::make_shared<VersionFile>();
			file->addProblem(ProblemSeverity::Error, QObject::tr("Minecraft version is missing in the FTB data."));
		}
		file->uid = "net.minecraft";
		file->name = QObject::tr("Minecraft (tracked)");
		if(file->version.isEmpty())
		{
			file->version = mcVersion;
		}
		minecraftPatch = std::make_shared<ProfilePatch>(file);
		minecraftPatch->setVanilla(true);
		minecraftPatch->setOrder(-2);
	}
	profile->appendPatch(minecraftPatch);

	ProfilePatchPtr packPatch;
	{
		std::shared_ptr< VersionFile > file;
		auto mcJson = m_instance->minecraftRoot() + "/pack.json";
		// load up the base minecraft patch, if it's there...
		if(QFile::exists(mcJson))
		{
			file = ProfileUtils::parseJsonFile(QFileInfo(mcJson), false);
			// adapt the loaded file - the FTB patch file format is different than ours.
			file->minecraftVersion.clear();
			file->mainJar = nullptr;
			for(auto addLib: file->libraries)
			{
				addLib->setHint("local");
				addLib->setStoragePrefix(nativeInstance->librariesPath().absolutePath());
			}
		}
		else
		{
			file = std::make_shared<VersionFile>();
			file->addProblem(ProblemSeverity::Error, QObject::tr("Modpack version file is missing."));
		}
		file->uid = "org.multimc.ftb.pack";
		file->name = QObject::tr("%1 (FTB pack)").arg(m_instance->name());
		if(file->version.isEmpty())
		{
			file->version = QObject::tr("Unknown");
			QFile versionFile (FS::PathCombine(m_instance->instanceRoot(), "version"));
			if(versionFile.exists())
			{
				if(versionFile.open(QIODevice::ReadOnly))
				{
					// FIXME: just guessing the encoding/charset here.
					auto version = QString::fromUtf8(versionFile.readAll());
					file->version = version;
				}
			}
		}
		packPatch = std::make_shared<ProfilePatch>(file);
		packPatch->setVanilla(true);
		packPatch->setOrder(1);
	}
	profile->appendPatch(packPatch);
}

void FTBProfileStrategy::load()
{
	profile->clearPatches();

	loadDefaultBuiltinPatches();
	loadUserPatches();
}

bool FTBProfileStrategy::saveOrder(ProfileUtils::PatchOrder order)
{
	return false;
}

bool FTBProfileStrategy::resetOrder()
{
	return false;
}

bool FTBProfileStrategy::installJarMods(QStringList filepaths)
{
	return false;
}

bool FTBProfileStrategy::customizePatch(ProfilePatchPtr patch)
{
	return false;
}

bool FTBProfileStrategy::revertPatch(ProfilePatchPtr patch)
{
	return false;
}
