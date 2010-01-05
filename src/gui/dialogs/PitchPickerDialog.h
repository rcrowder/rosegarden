
/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2010 the Rosegarden development team.

    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _RG_PITCHPICKERDIALOG_H_
#define _RG_PITCHPICKERDIALOG_H_

#include "gui/widgets/PitchChooser.h"
#include <QDialog>


class QWidget;


namespace Rosegarden
{

class PitchPickerDialog : public QDialog
{
    Q_OBJECT

public:

    PitchPickerDialog(QWidget* parent, int initialPitch, QString text);
    ~PitchPickerDialog();

    int getPitch() { return m_pitch->getPitch(); }
    
private:
    PitchChooser* m_pitch;
};


}

#endif
