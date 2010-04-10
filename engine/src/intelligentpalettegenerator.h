/*
  Q Light Controller
  intelligentpalettegenerator.h

  Copyright (C) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef INTELLIGENTPALETTEGENERATOR_H
#define INTELLIGENTPALETTEGENERATOR_H

#include <QString>
#include <QList>
#include <QHash>

class Fixture;
class Scene;
class Doc;

class IntelligentPaletteGenerator
{
public:
	IntelligentPaletteGenerator(Doc* doc, const QList <Fixture*>& fxiList);
	~IntelligentPaletteGenerator();

	void createColours();
	void createGobos();
	void createShutters();

	/**
	 * Create scenes for each capability found from fixture's channels
	 * that belong to the given channel group. For example, if $group
	 * is "Colour" and $fxi has a colour wheel that has red, green, blue
	 * and cyan colours, this generates 4 scenes; one for each colour.
	 *
	 * All fixtures that contain similar features end up eventually in the
	 * same scene so that running a "Cyan" scene will set all fixtures'
	 * colour channels to Cyan value.
	 *
	 * @param fxi The fixture instance to create functions for
	 * @param group
	 */
	void createGroupScenes(const Fixture* fxi, const QString& group);

protected:
	/** Find fixture channels that belong to a certain group and have more
	    than one capability (i.e. there's something make a palette from) */
	QList <t_channel> findChannels(const Fixture* fixture,
				       const QString& group) const;

	/** Add generated scenes to Doc */
	void addScenesToDoc();

	/** Delete all scenes that haven't been added to Doc */
	void purgeScenes();

private:
	Doc* m_doc;
	QList <Fixture*> m_fixtures;
	QHash <QString,Scene*> m_scenes;
};

#endif

