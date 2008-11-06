
/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2008 the Rosegarden development team.

    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _RG_MATRIXVELOCITY_H_
#define _RG_MATRIXVELOCITY_H_

#include "MatrixTool.h"
#include <qstring.h>
#include "base/Event.h"


class QMouseEvent;


namespace Rosegarden
{

class ViewElement;
class MatrixView;
class MatrixStaff;
class MatrixElement;
class Event;


class MatrixVelocity : public MatrixTool
{
    Q_OBJECT

    friend class MatrixToolBox;

public:
    virtual void handleLeftButtonPress(timeT,
                                       int height,
                                       int staffNo,
                                       QMouseEvent *event,
                                       ViewElement*);

    /**
     * Set the velocity of the element
     */
    virtual int handleMouseMove(timeT,
                                int height,
                                QMouseEvent*);

    /**
     * Actually update the velocity of the element
     */
    virtual void handleMouseRelease(timeT,
                                    int height,
                                    QMouseEvent*);

    static const QString ToolName;

    /**
     * Respond to an event being deleted -- it may be the one the tool
     * is remembering as the current event.
     */
    virtual void handleEventRemoved(Event *event);

    virtual void ready();
    virtual void stow();

protected slots:

    void slotMatrixScrolled(int x, int y);

protected:
    int m_velocityDelta;
    int m_screenPixelsScale;						// Amount of screen pixels  used for scale +-127 1:1 scale ratio
    int m_mouseStartY;
    double m_velocityScale;
    MatrixVelocity(MatrixView*);

    void setBasicContextHelp();

    MatrixElement* m_currentElement;
    MatrixStaff* m_currentStaff;
};



}

#endif