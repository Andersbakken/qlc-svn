/*
  Q Light Controller
  qlcinplugin.h

  Copyright (c) Heikki Junnila

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

#ifndef QLCINPLUGIN_H
#define QLCINPLUGIN_H

#include <QStringList>
#include <QtPlugin>
#include <QString>

#include "qlctypes.h"

class QObject;

/*****************************************************************************
 * InputPlugin
 *****************************************************************************/

/**
 * QLCInPlugin is an interface for all input plugins. Input plugins provide
 * input data from various (external) gadgets, such as slider wings, for
 * example.
 *
 * Each plugin can be understood as an adaptation layer between QLC and a
 * protocol used by a certain family of input devices. For example, a MIDI
 * input plugin can provide input support for a multitude of various MIDI
 * devices. Each of the devices currently connected to the computer can then
 * be represented as a separate input line by the MIDI plugin. Each plugin
 * must provide at least one input line for QLC in order to work properly.
 * Then again, if there are no such devices currently connected to the computer
 * that would be supported by the plugin, the plugin can choose to provide no
 * lines at all (until the user plugs in a supported device).
 *
 * When QLC has successfully loaded an input plugin, it will call init()
 * exactly once for each plugin. After that, it is assumed that either the
 * plugin auto-senses the devices it supports or the user must manually try
 * to search for new devices thru a custom configuration dialog that can be
 * opened with configure(). connectInputData() is also called exactly once
 * for each plugin to make a signal-slot connection between the plugin and
 * QLC.
 *
 * Plugins should not leave any resources open unless open() is called. And
 * even then, the plugin should open only such resources that are needed for
 * the specific input line given in the call to open(). Respectively, when
 * close() is called, the plugin should relinquish all resources associated to
 * the closed input line (unless shared with other lines).
 *
 * Plugins have a name that is shown to users as a list item. Each input
 * line name, preceded by its index, is shown under the plugin name as a
 * selectable list entry. Therefore, these names should be descriptive, but
 * relatively short. Input line indices are shown on the UI as 1-based, but
 * they are still handled internally as 0-based.
 *
 * An info text can be fetched for each plugin with infoText(). If the input
 * parameter == KInputInvalid, the plugin should provide a brief status snippet
 * on its overall state. If the input line parameter is given, the plugin
 * should provide information concerning ONLY that particular input line.
 * This info is displayed to the user as-is.
 *
 * Feedback values going the other way from QLC towards input devices is done
 * with feedbackData(). This data can be used by the input devices for e.g.
 * moving motorized faders to reflect their status on the QLC UI.
 */
class QLCInPlugin
{
public:
    /**
     * De-initialize the plugin. This is the last thing that is called
     * for the plugin so make sure nothing is lingering in the twilight
     * after this call.
     */
    virtual ~QLCInPlugin() {}

    /**
     * Initialize the plugin. Since plugins cannot have a user-defined
     * constructor, any initialization prior to opening any HW must be
     * done thru this second-stage initialization method. InputMap calls
     * this function for all plugins exactly once after loading, before
     * calling any other method from the plugin.
     *
     * This is a pure virtual function that must be implemented in all
     * plugins.
     */
    virtual void init() = 0;

    /**
     * Open the specified input line so that the plugin can start
     * sending input data from that line.
     *
     * @param input The input line to open
     */
    virtual void open(quint32 input = 0) = 0;

    /**
     * Close the specified input line so that the plugin can stop
     * sending input data from that line.
     *
     * @param input The input line to close
     */
    virtual void close(quint32 input = 0) = 0;

    /**
     * Invoke a configuration dialog for the plugin
     *
     * This is a pure virtual function that must be implemented
     * in all plugins.
     */
    virtual void configure() = 0;

    /**
     * Provide an information text to be displayed in the plugin manager
     *
     * This is a pure virtual function that must be implemented
     * in all plugins.
     *
     * @param input If specified, information for the given input line is
     *              expected. Otherwise provides information for the plugin
     */
    virtual QString infoText(quint32 input = KInputInvalid) = 0;

    /**
     * Get the plugin's name
     */
    virtual QString name() = 0;

    /*********************************************************************
     * Inputs
     *********************************************************************/
public:
    /**
     * Get a list of input lines' names. The names must be always in the
     * same order i.e. the first name is the name of input line number 0,
     * the next one is input line number 1, etc.. These indices are used
     * with open() and close().
     *
     * @return A list of available input names
     */
    virtual QStringList inputs() = 0;

    /*********************************************************************
     * Input data listener
     *********************************************************************/
public:
    /**
     * Normally, if QLCInPlugin were a QObject, one would just define a
     * signal to QLCInPlugin that is emitted when input data is available.
     * It seems, however, that plugins' base interface class (QLCInPlugin)
     * cannot be a QObject, so the basic signal-slot approach cannot be used.
     *
     * Each input plugin implementation must:
     *    1. Inherit QObject and QLCInPlugin
     *    2. Re-implement this method and connect the plugin's signals:
     *
     *         - valueChanged(QLCInPlugin* plugin, quint32 line,
     *                        quint32 ch, uchar val);
     *         - configurationChanged();
     *
     * to the given listener object's slots:
     *
     *         - slotValueChanged(QLCInPlugin* plugin, quint32 line,
     *                            quint32 ch, uchar val);
     *         - slotConfigurationChanged();
     *
     * With this approach, input plugins can use the signal-slot mechanism
     * just as if QLCInPlugin inherited QObject directly.
     */
    virtual void connectInputData(QObject* listener) = 0;

    /*********************************************************************
     * Feedback
     *********************************************************************/
public:
    /**
     * Send a value back to an input line's channel. This method can be
     * used for example to move motorized sliders with QLC sliders. If the
     * hardware /that the plugin provides access to) doesn't support this,
     * the implementation can be left empty.
     *
     * @param input The input line to send feedback to
     * @param channel A channel in the input line to send feedback to
     * @param value An input value to send back to the input channel
     */
    virtual void feedBack(quint32 input, quint32 channel,
                          uchar value) = 0;
};

Q_DECLARE_INTERFACE(QLCInPlugin, "QLCInPlugin")

#endif
