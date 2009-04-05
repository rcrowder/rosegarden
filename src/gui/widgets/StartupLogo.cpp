// -*- c-basic-offset: 4 -*-

/*
    Rosegarden
    A sequencer and musical notation editor.
    Copyright 2000-2009 the Rosegarden development team.
    See the AUTHORS file for more details.

    This file contains code borrowed from KDevelop 2.0
    Copyright (c) The KDevelop Development Team.
 
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/


#include "StartupLogo.h"

#include "misc/Debug.h"
#include "gui/general/IconLoader.h"
#include "gui/general/ResourceFinder.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QFontMetrics>
#include <QSettings>

#include <unistd.h>

namespace Rosegarden
{

// NOTE: use QSplashScreen instead ??

StartupLogo::StartupLogo(QWidget * parent, const char *name) :
//    QWidget(parent, name, Qt::SplashScreen),
    QWidget(parent, Qt::SplashScreen),
    m_readyToHide(false),
    m_showTip(true)
{
    setObjectName("Splash");
    m_pixmap = IconLoader().loadPixmap("splash");
    //setBackgroundPixmap(m_pixmap); This is Qt3 support - doesn't work with style sheets
    setGeometry(QApplication::desktop()->width() / 2 - m_pixmap.width() / 2,
                QApplication::desktop()->height() / 2 - m_pixmap.height() / 2,
                m_pixmap.width(), m_pixmap.height());
}

StartupLogo::~StartupLogo()
{
    m_wasClosed = true;
    m_instance = 0;
}

void StartupLogo::paintEvent(QPaintEvent*)
{
    // Print version number
    QPainter paint(this);

    // Draw the splash screen image
    paint.drawPixmap(0,0,m_pixmap);
    
    QFont defaultFont;
    defaultFont.setPixelSize(12);
    paint.setFont(defaultFont);

    QFontMetrics metrics(defaultFont);
    int width = metrics.width(m_statusMessage) + 6;
    if (width > 200)
        width = 200;

    int y = m_pixmap.height() - 12;

    // removed: why paint a colored rectangle to draw on instead of just drawing
    // on the splash directly?

    paint.setPen(QColor(Qt::white));
    paint.setBrush(QColor(Qt::white));

    //QString version(VERSION);
    //int sepIdx = version.find("-");
    QString versionLabel(VERSION);
    //QString("R%1 v%2").arg(version.left(sepIdx)).arg(version.mid(sepIdx + 1));
    int versionWidth = metrics.width(versionLabel);

    paint.drawText(m_pixmap.width() - versionWidth - 18,
                   m_pixmap.height() - 28,
                   versionLabel);

    paint.drawText(m_pixmap.width() - (width + 10), y, m_statusMessage);
}

void StartupLogo::slotShowStatusMessage(QString message)
{
    m_statusMessage = message;
    paintEvent(0);
    QApplication::flushX();
}

void StartupLogo::close()
{
    if (!m_wasClosed && isVisible()) {

	if (m_showTip) {
	    QString tipResource = ResourceFinder().getResourcePath("", "tips");
	    if (tipResource != "") {
		RG_DEBUG << "StartupLogo::close: Showing Tips\n";
//&&&		KTipDialog::showTip(tipResource);
	    }
	}
    }

    QWidget::close();
}


void StartupLogo::mousePressEvent(QMouseEvent*)
{
    // for the haters of raising startlogos
    if (m_readyToHide)
        hide(); // don't close, main() sets up a QTimer for that
}

StartupLogo* StartupLogo::getInstance()
{
    if (m_wasClosed)
        return 0;

    if (!m_instance)
        m_instance = new StartupLogo;

    return m_instance;
}

void StartupLogo::hideIfStillThere()
{
    if (m_instance)
        m_instance->hide();
    // don't close, main() sets up a QTimer for that
}


StartupLogo* StartupLogo::m_instance = 0;
bool StartupLogo::m_wasClosed = false;

#include "StartupLogo.moc"

}
