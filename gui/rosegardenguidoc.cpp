/***************************************************************************
                          rosegardenguidoc.cpp  -  description
                             -------------------
    begin                : Mon Jun 19 23:41:03 CEST 2000
    copyright            : (C) 2000 by Guillaume Laurent, Chris Cannam, Rich Bown
    email                : glaurent@telegraph-road.org, cannam@all-day-breakfast.com, bownie@bownie.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <iostream>

// include files for Qt
#include <qdir.h>
#include <qfileinfo.h>
#include <qwidget.h>

// include files for KDE
#include <klocale.h>
#include <kmessagebox.h>

// application specific includes
#include "rosedebug.h"
#include "rosegardenguidoc.h"
#include "rosegardengui.h"
#include "rosegardenguiview.h"
#include "rosexmlhandler.h"
#include "viewelementsmanager.h"
#include "xmlstorableevent.h"
#include "Element2.h"

QList<RosegardenGUIView> *RosegardenGUIDoc::pViewList = 0L;

RosegardenGUIDoc::RosegardenGUIDoc(QWidget *parent, const char *name)
    : QObject(parent, name),
      m_viewElementsManager(0)
{
    if(!pViewList) {
        pViewList = new QList<RosegardenGUIView>();
    }

    pViewList->setAutoDelete(true);

    m_viewElementsManager = new ViewElementsManager(m_composition);
}

RosegardenGUIDoc::~RosegardenGUIDoc()
{
}

void RosegardenGUIDoc::addView(RosegardenGUIView *view)
{
    pViewList->append(view);
}

void RosegardenGUIDoc::removeView(RosegardenGUIView *view)
{
    pViewList->remove(view);
}

void RosegardenGUIDoc::setAbsFilePath(const QString &filename)
{
    m_absFilePath=filename;
}

const QString &RosegardenGUIDoc::getAbsFilePath() const
{
    return m_absFilePath;
}

void RosegardenGUIDoc::setTitle(const QString &_t)
{
    m_title=_t;
}

const QString &RosegardenGUIDoc::getTitle() const
{
    return m_title;
}

void RosegardenGUIDoc::slotUpdateAllViews(RosegardenGUIView *sender)
{
    RosegardenGUIView *w;
    if(pViewList)
        {
            for(w=pViewList->first(); w!=0; w=pViewList->next())
                {
                    if(w!=sender)
                        w->repaint();
                }
        }

}

bool RosegardenGUIDoc::saveIfModified()
{
    bool completed=true;

    if(m_modified) {
        RosegardenGUIApp *win=(RosegardenGUIApp *) parent();
        int want_save = KMessageBox::warningYesNoCancel(win, i18n("Warning"),
                                                        i18n("The current file has been modified.\n"
                                                             "Do you want to save it?"));
        switch(want_save)
            {
            case 1:
                if (m_title == i18n("Untitled")) {
                    win->slotFileSaveAs();
                } else {
                    saveDocument(getAbsFilePath());
                };

                deleteContents();
                completed=true;
                break;

            case 2:
                setModified(false);
                deleteContents();
                completed=true;
                break;	

            case 3:
                completed=false;
                break;

            default:
                completed=false;
                break;
            }
    }

    return completed;
}

void RosegardenGUIDoc::closeDocument()
{
    deleteContents();
}

bool RosegardenGUIDoc::newDocument()
{
    /////////////////////////////////////////////////
    // TODO: Add your document initialization code here
    /////////////////////////////////////////////////
    m_modified=false;
    m_absFilePath=QDir::homeDirPath();
    m_title=i18n("Untitled");

    return true;
}

bool RosegardenGUIDoc::openDocument(const QString &filename,
                                    const char* /*format*/ /*=0*/)
{
    kdDebug(KDEBUG_AREA) << "RosegardenGUIDoc::openDocument("
                         << filename << ")" << endl;
    
    if (!filename || filename.isEmpty())
        return false;

    QFileInfo fileInfo(filename);
    m_title=fileInfo.fileName();

    // Check if file readable with fileInfo ?
    if (!fileInfo.isReadable()) {
        QString msg(i18n("Can't open file '"));
        msg += filename;
        msg += "'";
        
        KMessageBox::sorry(0, msg);

        return false;
    }
    
    m_absFilePath=fileInfo.absFilePath();	

    QFile file(filename);
    bool fileParsedOk = xmlParse(file);
    file.close();   

    if (! fileParsedOk) {
        QString msg(i18n("Error when parsing file '"));
        msg += filename;
        msg += "'";
        
        KMessageBox::sorry(0, msg);

        return false;
    }

    m_viewElementsManager = new ViewElementsManager(m_composition);

    return true;
}

bool RosegardenGUIDoc::saveDocument(const QString &filename,
                                    const char* /*format*/ /*=0*/)
{
    kdDebug(KDEBUG_AREA) << "RosegardenGUIDoc::saveDocument("
                         << filename << ")" << endl;

    QFile file(filename);
    file.open(IO_WriteOnly);

    QTextStream fileStream(&file);

    // output XML header
    //
    fileStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
               << "<!DOCTYPE rosegarden-data>\n"
               << "<rosegarden-data>\n";

    // output all elements
    //
    // Iterate on tracks
    for(Composition::iterator trks = m_composition.begin();
        trks != m_composition.end(); ++trks) {

        fileStream << "<Track>" << endl; //--------------------------

        for(EventList::iterator i = (*trks)->begin();
            i != (*trks)->end(); ++i) {

            EventList::iterator nextEl = i;
            ++nextEl;

            if (nextEl != (*trks)->end()) {

                Event::timeT absTime = (*i)->absoluteTime();
            
                if ((*nextEl)->absoluteTime() == absTime) {
                    // group elements
                    //
                    fileStream << "<Group>" << endl; //------

                    while ((*i)->absoluteTime() == absTime) {
                        fileStream << '\t'
                                   << XMLStorableEvent::toXMLString(*(*i))
                                   << endl;
                        ++i;
                    }

                    fileStream << "</Group>" << endl; //-----

                    if (i == (*trks)->end()) break;
                }
            }
        
            fileStream << XMLStorableEvent::toXMLString(*(*i)) << endl;
        }

        fileStream << "</Track>" << endl; //-------------------------

    }
    
    // close the top-level XML tag
    //
    fileStream << "</rosegarden-data>\n";

    kdDebug(KDEBUG_AREA) << endl << "RosegardenGUIDoc::saveDocument() finished"
                         << endl;

    m_modified=false;
    return true;
}

void RosegardenGUIDoc::deleteContents()
{
    kdDebug(KDEBUG_AREA) << "RosegardenGUIDoc::deleteContents()" << endl;

    deleteViews();

    for(Composition::iterator trks = m_composition.begin();
        trks != m_composition.end(); ++trks) {

        for(EventList::iterator i = (*trks)->begin();
            i != (*trks)->end(); ++i) {
            delete *i;
        }

        (*trks)->clear();

    }
    
    m_composition.clear();

    delete m_viewElementsManager;
    m_viewElementsManager = 0;
}

void RosegardenGUIDoc::deleteViews()
{
    // auto-deletion is enabled : GUIViews will be deleted
    pViewList->clear();
}


//            1 unit  = 1/4 clock
//            4 units =   1 clock
//            6 units = 3/2 clocks = 1 hemidemisemiquaver
//           12 units =   3 clocks = 1 demisemiquaver
//           24 units =   6 clocks = 1 semiquaver
//           48 units =  12 clocks = 1 quaver
//           96 units =  24 clocks = 1 crotchet
//          192 units =  48 clocks = 1 minim
//          384 units =  96 clocks = 1 semibreve
//          768 units = 192 clocks = 1 breve


bool
RosegardenGUIDoc::xmlParse(QFile &file)
{
    // parse xml file
    RoseXmlHandler handler(m_composition);

    QXmlInputSource source(file);
    QXmlSimpleReader reader;
    reader.setContentHandler(&handler);
    reader.setErrorHandler(&handler);

    bool ok = reader.parse(source);


    // m_doc.setContent(xmldata);
    
//     QDomElement baseNode = m_doc.documentElement();

//     xmlParseElement(baseNode);

    return ok;
}
