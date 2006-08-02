// -*- c-basic-offset: 4 -*-

/*
    Rosegarden-4
    A sequencer and musical notation editor.

    This program is Copyright 2000-2006
        Guillaume Laurent   <glaurent@telegraph-road.org>,
        Chris Cannam        <cannam@all-day-breakfast.com>,
        Richard Bown        <bownie@bownie.com>

    The moral right of the authors to claim authorship of this work
    has been asserted.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/


#ifndef _TEMPORULER_H_
#define _TEMPORULER_H_

#include <qwidget.h>
#include "Event.h"
#include "Composition.h"

namespace Rosegarden {
    class RulerScale;
}

class QFont;
class QFontMetrics;
class RosegardenGUIDoc;
class RosegardenTextFloat;


/**
 * TempoRuler is a widget that shows a strip of tempo values at
 * x-coordinates corresponding to tempo changes in a Composition.
 */

class TempoRuler : public QWidget
{
    Q_OBJECT

public:
    /**
     * Construct a TempoRuler that displays the tempo changes found
     * in the given Composition at positions calculated by the given
     * RulerScale.
     *
     * The RulerScale will not be destroyed along with the TextRuler.
     */
    TempoRuler(Rosegarden::RulerScale *rulerScale,
	       RosegardenGUIDoc *doc,
	       double xorigin = 0.0,
	       int height = 0,
	       bool small = false,
	       QWidget* parent = 0,
	       const char *name = 0);

    ~TempoRuler();

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

    void setMinimumWidth(int width) { m_width = width; }

signals:
    void doubleClicked(Rosegarden::timeT);

public slots:
    void slotScrollHoriz(int x);

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void wheelEvent(QWheelEvent *);

private:
    double m_xorigin;
    int  m_height;
    int  m_currentXOffset;
    int  m_width;
    bool m_small;
    int  m_illuminate;
    bool m_refreshLinesOnly;

    bool m_dragging;
    int  m_dragStartY;

    int getYForTempo(Rosegarden::tempoT tempo);
    Rosegarden::tempoT getTempoForY(int y);
    void showTextFloat(Rosegarden::tempoT tempo);

    Rosegarden::Composition *m_composition;
    Rosegarden::RulerScale *m_rulerScale;
    RosegardenTextFloat *m_textFloat;

    QFont m_font;
    QFont m_boldFont;
    QFontMetrics m_fontMetrics;
};

#endif // _TEMPORULER_H_

