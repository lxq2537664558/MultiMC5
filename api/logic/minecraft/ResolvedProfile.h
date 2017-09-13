#pragma once
#include <QString>
#include <QList>
#include "Library.h"
#include <ProblemProvider.h>

class VersionFile;

class ResolvedProfile
{
public:
	void getLibraryFiles(const QString & architecture, QStringList & jars, QStringList & nativeJars, const QString & overridePath,
		const QString & tempPath) const;
	MojangAssetIndexInfo::Ptr getMinecraftAssets() const;
	bool hasTrait(const QString & trait) const;

	void clear()
	{
		m_minecraftVersion.clear();
		m_minecraftVersionType.clear();
		m_minecraftAssets.reset();
		m_minecraftArguments.clear();
		m_tweakers.clear();
		m_mainClass.clear();
		m_appletClass.clear();
		m_libraries.clear();
		m_traits.clear();
		m_jarMods.clear();
		m_mainJar.reset();
		m_problemSeverity = ProblemSeverity::None;
	}

	void apply(VersionFile *patch);

private:
	void applyMinecraftVersion(const QString& id);
	void applyMainClass(const QString& mainClass);
	void applyAppletClass(const QString& appletClass);
	void applyMinecraftArguments(const QString& minecraftArguments);
	void applyMinecraftVersionType(const QString& type);
	void applyMinecraftAssets(MojangAssetIndexInfo::Ptr assets);
	void applyTraits(const QSet<QString> &traits);
	void applyTweakers(const QStringList &tweakers);
	void applyJarMods(const QList<LibraryPtr> &jarMods);
	void applyMods(const QList<LibraryPtr> &jarMods);
	void applyLibrary(LibraryPtr library);
	void applyMainJar(LibraryPtr jar);
	void applyProblemSeverity(ProblemSeverity severity);

public: /* data */
	/// the version of Minecraft - jar to use
	QString m_minecraftVersion;

	/// Release type - "release" or "snapshot"
	QString m_minecraftVersionType;

	/// Assets type - "legacy" or a version ID
	MojangAssetIndexInfo::Ptr m_minecraftAssets;

	/**
	 * arguments that should be used for launching minecraft
	 *
	 * ex: "--username ${auth_player_name} --session ${auth_session}
	 *      --version ${version_name} --gameDir ${game_directory} --assetsDir ${game_assets}"
	 */
	QString m_minecraftArguments;

	/// A list of all tweaker classes
	QStringList m_tweakers;

	/// The main class to load first
	QString m_mainClass;

	/// The applet class, for some very old minecraft releases
	QString m_appletClass;

	/// the list of libraries
	QList<LibraryPtr> m_libraries;

	/// the main jar
	LibraryPtr m_mainJar;

	/// the list of libraries
	QList<LibraryPtr> m_nativeLibraries;

	/// traits, collected from all the version files (version files can only add)
	QSet<QString> m_traits;

	/// A list of jar mods. version files can add those.
	QList<LibraryPtr> m_jarMods;

	/// the list of mods
	QList<LibraryPtr> m_mods;

	ProblemSeverity m_problemSeverity = ProblemSeverity::None;
};
