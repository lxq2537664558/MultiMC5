
#include "ResolvedProfile.h"
#include "VersionFile.h"
#include "Version.h"

static bool isMinecraftVersion(const QString &uid)
{
	return uid == "net.minecraft";
}

void ResolvedProfile::apply(VersionFile* patch)
{
	// Only real Minecraft can set those. Don't let anything override them.
	if (isMinecraftVersion(patch->uid))
	{
		applyMinecraftVersion(patch->minecraftVersion);
		applyMinecraftVersionType(patch->type);
		// HACK: ignore assets from other version files than Minecraft
		// workaround for stupid assets issue caused by amazon:
		// https://www.theregister.co.uk/2017/02/28/aws_is_awol_as_s3_goes_haywire/
		applyMinecraftAssets(patch->mojangAssetIndex);
	}

	applyMainJar(patch->mainJar);
	applyMainClass(patch->mainClass);
	applyAppletClass(patch->appletClass);
	applyMinecraftArguments(patch->minecraftArguments);
	applyTweakers(patch->addTweakers);
	applyJarMods(patch->jarMods);
	applyMods(patch->mods);
	applyTraits(patch->traits);

	for (auto library : patch->libraries)
	{
		applyLibrary(library);
	}
	applyProblemSeverity(patch->getProblemSeverity());
}


static void applyString(const QString & from, QString & to)
{
	if(from.isEmpty())
		return;
	to = from;
}

void ResolvedProfile::applyMinecraftVersion(const QString& id)
{
	applyString(id, this->m_minecraftVersion);
}

void ResolvedProfile::applyAppletClass(const QString& appletClass)
{
	applyString(appletClass, this->m_appletClass);
}

void ResolvedProfile::applyMainClass(const QString& mainClass)
{
	applyString(mainClass, this->m_mainClass);
}

void ResolvedProfile::applyMinecraftArguments(const QString& minecraftArguments)
{
	applyString(minecraftArguments, this->m_minecraftArguments);
}

void ResolvedProfile::applyMinecraftVersionType(const QString& type)
{
	applyString(type, this->m_minecraftVersionType);
}

void ResolvedProfile::applyMinecraftAssets(MojangAssetIndexInfo::Ptr assets)
{
	if(assets)
	{
		m_minecraftAssets = assets;
	}
}

void ResolvedProfile::applyTraits(const QSet<QString>& traits)
{
	this->m_traits.unite(traits);
}

void ResolvedProfile::applyTweakers(const QStringList& tweakers)
{
	// FIXME: check for dupes?
	// FIXME: does order matter?
	for (auto tweaker : tweakers)
	{
		this->m_tweakers += tweaker;
	}
}

void ResolvedProfile::applyJarMods(const QList<LibraryPtr>& jarMods)
{
	this->m_jarMods.append(jarMods);
}

static int findLibraryByName(QList<LibraryPtr> *haystack, const GradleSpecifier &needle)
{
	int retval = -1;
	for (int i = 0; i < haystack->size(); ++i)
	{
		if (haystack->at(i)->rawName().matchName(needle))
		{
			// only one is allowed.
			if (retval != -1)
				return -1;
			retval = i;
		}
	}
	return retval;
}

void ResolvedProfile::applyMods(const QList<LibraryPtr>& mods)
{
	QList<LibraryPtr> * list = &m_mods;
	for(auto & mod: mods)
	{
		auto modCopy = Library::limitedCopy(mod);

		// find the mod by name.
		const int index = findLibraryByName(list, mod->rawName());
		// mod not found? just add it.
		if (index < 0)
		{
			list->append(modCopy);
			return;
		}

		auto existingLibrary = list->at(index);
		// if we are higher it means we should update
		if (Version(mod->version()) > Version(existingLibrary->version()))
		{
			list->replace(index, modCopy);
		}
	}
}

void ResolvedProfile::applyLibrary(LibraryPtr library)
{
	if(!library->isActive())
	{
		return;
	}

	QList<LibraryPtr> * list = &m_libraries;
	if(library->isNative())
	{
		list = &m_nativeLibraries;
	}

	auto libraryCopy = Library::limitedCopy(library);

	// find the library by name.
	const int index = findLibraryByName(list, library->rawName());
	// library not found? just add it.
	if (index < 0)
	{
		list->append(libraryCopy);
		return;
	}

	auto existingLibrary = list->at(index);
	// if we are higher it means we should update
	if (Version(library->version()) > Version(existingLibrary->version()))
	{
		list->replace(index, libraryCopy);
	}
}

void ResolvedProfile::applyMainJar(LibraryPtr jar)
{
	if(jar)
	{
		m_mainJar = jar;
	}
}

void ResolvedProfile::applyProblemSeverity(ProblemSeverity severity)
{
	if (m_problemSeverity < severity)
	{
		m_problemSeverity = severity;
	}
}

void ResolvedProfile::getLibraryFiles(const QString& architecture, QStringList& jars, QStringList& nativeJars, const QString& overridePath, const QString& tempPath) const
{
	QStringList native32, native64;
	jars.clear();
	nativeJars.clear();
	for (auto lib : m_libraries)
	{
		lib->getApplicableFiles(currentSystem, jars, nativeJars, native32, native64, overridePath);
	}
	// NOTE: order is important here, add main jar last to the lists
	if(m_mainJar)
	{
		// FIXME: HACK!! jar modding is weird and unsystematic!
		if(m_jarMods.size())
		{
			QDir tempDir(tempPath);
			jars.append(tempDir.absoluteFilePath("minecraft.jar"));
		}
		else
		{
			m_mainJar->getApplicableFiles(currentSystem, jars, nativeJars, native32, native64, overridePath);
		}
	}
	for (auto lib : m_nativeLibraries)
	{
		lib->getApplicableFiles(currentSystem, jars, nativeJars, native32, native64, overridePath);
	}
	if(architecture == "32")
	{
		nativeJars.append(native32);
	}
	else if(architecture == "64")
	{
		nativeJars.append(native64);
	}
}

bool ResolvedProfile::hasTrait(const QString& trait) const
{
	return m_traits.contains(trait);
}

std::shared_ptr<MojangAssetIndexInfo> ResolvedProfile::getMinecraftAssets() const
{
	if(!m_minecraftAssets)
	{
		return std::make_shared<MojangAssetIndexInfo>("legacy");
	}
	return m_minecraftAssets;
}
