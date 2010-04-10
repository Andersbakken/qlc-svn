#include <QString>

#include "qlccapability.h"
#include "qlcchannel.h"

#include "intelligentpalettegenerator.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"

IntelligentPaletteGenerator::IntelligentPaletteGenerator(Doc* doc,
						const QList <Fixture*>& fxiList)
	: m_doc(doc)
	, m_fixtures(fxiList)
{
}

IntelligentPaletteGenerator::~IntelligentPaletteGenerator()
{
	purgeScenes();
}

void IntelligentPaletteGenerator::createColours()
{
	purgeScenes();

	/* Create functions for each selected fixture */
	QListIterator <Fixture*> fxiit(m_fixtures);
	while (fxiit.hasNext() == true)
	{
		Fixture* fxi(fxiit.next());
		Q_ASSERT(fxi != NULL);

		createGroupScenes(fxi, KQLCChannelGroupColour);
	}

	addScenesToDoc();
}

void IntelligentPaletteGenerator::createGobos()
{
	purgeScenes();

	/* Create functions for each selected fixture */
	QListIterator <Fixture*> fxiit(m_fixtures);
	while (fxiit.hasNext() == true)
	{
		Fixture* fxi(fxiit.next());
		Q_ASSERT(fxi != NULL);

		createGroupScenes(fxi, KQLCChannelGroupGobo);
	}

	addScenesToDoc();
}

void IntelligentPaletteGenerator::createShutters()
{
	purgeScenes();

	/* Create functions for each selected fixture */
	QListIterator <Fixture*> fxiit(m_fixtures);
	while (fxiit.hasNext() == true)
	{
		Fixture* fxi(fxiit.next());
		Q_ASSERT(fxi != NULL);

		createGroupScenes(fxi, KQLCChannelGroupShutter);
	}

	addScenesToDoc();
}

void IntelligentPaletteGenerator::createGroupScenes(const Fixture* fxi,
						    const QString& group)
{
	Q_ASSERT(fxi != NULL);
	Q_ASSERT(group.isEmpty() == false);

	QList <t_channel> channels = findChannels(fxi, group);
	for (int i = 0; i < channels.size(); i++)
	{
		t_channel ch = channels.at(i);
		const QLCChannel* channel = fxi->channel(ch);
		Q_ASSERT(channel != NULL);

		QListIterator <QLCCapability*> capit(channel->capabilities());
		while (capit.hasNext() == true)
		{
			const QLCCapability* cap = capit.next();
			Q_ASSERT(cap != NULL);

			QString name;
			if (channels.size() > 1)
			{
				// There's more than one channel (e.g. two gobo
				// gobo wheels, several distinct color wheels)
				name = QString("%1 (%2) - %3").arg(group)
						.arg(i + 1).arg(cap->name());
			}
			else
			{
				// There's only one channel of the group
				// in the fixture (e.g. one gobo wheel)
				name = QString("%1 - %2").arg(group)
							 .arg(cap->name());
			}

			Scene* scene = NULL;
			if (m_scenes.contains(name) == true)
			{
				// Append this fixture's values to a scene with the
				// same name, that contains other similar values.
				scene = m_scenes[name];
			}
			else
			{
				scene = new Scene(m_doc);
				scene->setName(name);
                                m_scenes[name] = scene;
			}

			scene->setValue(fxi->id(), ch, cap->middle());
		}
	}
}

QList <t_channel> IntelligentPaletteGenerator::findChannels(
			const Fixture* fixture, const QString& group) const
{
	QList <t_channel> channels;

	Q_ASSERT(fixture != NULL);
	for (t_channel ch = 0; ch < fixture->channels(); ch++)
	{
		const QLCChannel* channel(fixture->channel(ch));
		Q_ASSERT(channel != NULL);
		if (channel->group() == group && channel->capabilities().size() > 1)
			channels << ch;
	}

	return channels;
}

void IntelligentPaletteGenerator::purgeScenes()
{
	QHashIterator <QString,Scene*> it(m_scenes);
	while (it.hasNext() == true)
	{
		it.next();
		if (m_doc->function(it.value()->id()) == NULL)
			delete it.value();
	}
	m_scenes.clear();
}

void IntelligentPaletteGenerator::addScenesToDoc()
{
	QHashIterator <QString,Scene*> it(m_scenes);
	while (it.hasNext() == true)
	{
		it.next();
		if (m_doc->addFunction(it.value()) == false)
			break;
	}
}
