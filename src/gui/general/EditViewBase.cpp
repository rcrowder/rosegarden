/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2009 the Rosegarden development team.
 
    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.
 
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "EditViewBase.h"

#include "misc/Debug.h"
#include "base/Clipboard.h"
#include "base/Event.h"
#include "base/NotationTypes.h"
#include "base/Segment.h"
#include "commands/segment/SegmentReconfigureCommand.h"
#include "document/CommandHistory.h"
#include "document/RosegardenGUIDoc.h"
#include "EditToolBox.h"
#include "EditTool.h"
#include "EditView.h"
#include "gui/dialogs/ConfigureDialog.h"
#include "gui/dialogs/TimeDialog.h"
#include "gui/general/EditViewTimeSigNotifier.h"
#include "misc/Strings.h"
#include "gui/kdeext/KTmpStatusMsg.h"
#include "document/Command.h"

#include <QSettings>
#include <QDockWidget>
#include <QAction>
#include <QShortcut>
#include <QDialog>
#include <QFrame>
#include <QIcon>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QWidget>
#include <QStatusBar>
#include <QMainWindow>
#include <QCloseEvent>
#include <QLayout>
#include <QApplication>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QToolBar>

#include <Q3CanvasPixmap>
#include <Q3Canvas>




namespace Rosegarden
{

bool EditViewBase::m_inPaintEvent = false;
const unsigned int EditViewBase::ID_STATUS_MSG = 1;
const unsigned int EditViewBase::NbLayoutRows = 6;


EditViewBase::EditViewBase(RosegardenGUIDoc *doc,
                           std::vector<Segment *> segments,
                           unsigned int cols,
                           QWidget *parent, const char *name) :
    //KDockMainWindow(parent, name),
    QMainWindow(parent, name),
    m_viewNumber( -1),
    m_viewLocalPropertyPrefix(makeViewLocalPropertyPrefix()),
    m_doc(doc),
    m_segments(segments),
    m_tool(0),
    m_toolBox(0),
    m_mainDockWidget(0),
    m_centralFrame(0),
    m_grid(0),
    m_mainCol(cols - 1),
    m_compositionRefreshStatusId(doc->getComposition().getNewRefreshStatusId()),
    m_needUpdate(false),
    m_pendingPaintEvent(0),
    m_havePendingPaintEvent(false),
    m_shortcuts(0),
    m_configDialogPageIndex(0),
    m_inCtor(true),
    m_timeSigNotifier(new EditViewTimeSigNotifier(doc))
{
    QPixmap dummyPixmap; // any icon will do
	
	
	/*
	m_mainDockWidget = new QDockWidget( "Rosegarden EditView DockWidget", this );
	m_mainDockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
	m_mainDockWidget->setFeatures( QDockWidget::AllDockWidgetFeatures );
	
	addDockWidget( Qt::LeftDockWidgetArea, m_mainDockWidget, Qt::Horizontal );
	*/
	m_mainDockWidget = 0;
	
	setStatusBar( new QStatusBar(this) );
/*	
	m_toolBar = new QToolBar( "Tool Bar", this );
	addToolBar( Qt::TopToolBarArea, m_toolBar );
	m_toolBar->setMinimumHeight( 16 );
	m_toolBar->setMinimumWidth( 40 );
*/	
    m_centralFrame = new QFrame(this);		//m_mainDockWidget);
    m_centralFrame->setObjectName("centralframe");
	m_centralFrame->setMinimumSize( 500, 300 );
	m_centralFrame->setMaximumSize( 2200, 1400 );
	
	// 
	m_grid = new QGridLayout(m_centralFrame);
	m_centralFrame->setLayout( m_grid );
	
	// Note: We add Widget bottom-right, so the grid gets the propper col,row count
	// NbLayoutRows, cols
	//m_grid->addWidget( new QWidget(this), NbLayoutRows, cols);
	
	
	//this->setLayout( new QVBoxLayout(this) );
	//this->layout()->addWidget( m_centralFrame );
	setCentralWidget( m_centralFrame );
	
//    m_mainDockWidget->setWidget(m_centralFrame);

    initSegmentRefreshStatusIds();

    m_doc->attachEditView(this);

    QObject::connect
    (CommandHistory::getInstance(), SIGNAL(commandExecuted()),
     this, SLOT(update()));

    QObject::connect
    (CommandHistory::getInstance(), SIGNAL(commandExecuted()),
     this, SLOT(slotTestClipboard()));

    // create shortcuts
    //
    m_shortcuts = new QShortcut(this);
}

EditViewBase::~EditViewBase()
{
    delete m_timeSigNotifier;

    m_doc->detachEditView(this);

//&&& Detact CommandHistory
//    CommandHistory::getInstance()->detachView(actionCollection());
    m_viewNumberPool.erase(m_viewNumber);
    slotSaveOptions();
}

void EditViewBase::slotSaveOptions()
{}

void EditViewBase::readOptions()
{
    QAction *a = findAction("options_show_statusbar");
    if (a) a->setChecked( ! statusBar()->isHidden() );

//    a = findAction("options_show_toolbar");
//    if (a) a->setChecked( ! m_toolBar->isHidden());
}

void EditViewBase::setupActions(QString rcFileName, bool haveClipboard)
{
    setRCFileName(rcFileName);

    // Actions all edit views will have

//    createAction("options_show_toolbar", SLOT(slotToggleToolBar()));
    createAction("options_show_statusbar", SLOT(slotToggleStatusBar()));
    createAction("options_configure", SLOT(slotConfigure()));
//    createAction("options_configure_keybindings", SLOT(slotEditKeys()));
//    createAction("options_configure_toolbars", SLOT(slotEditToolbars()));

    createAction("file_save", SIGNAL(saveFile()));
    createAction("file_close", SLOT(slotCloseWindow()));


    if (haveClipboard) {
        createAction("edit_cut", SLOT(slotEditCut()));
        createAction("edit_copy", SLOT(slotEditCopy()));
        createAction("edit_paste", SLOT(slotEditPaste()));
    }

/*&&&
  Connect up CommandHistory appropriately

    new KToolBarPopupAction(tr("Und&o"),
                            "undo",
                            KStandardShortcut::key(KStandardShortcut::Undo),
                            actionCollection(),
                            KStandardAction::stdName(KStandardAction::Undo));

    new KToolBarPopupAction(tr("Re&do"),
                            "redo",
                            KStandardShortcut::key(KStandardShortcut::Redo),
                            actionCollection(),
                            KStandardAction::stdName(KStandardAction::Redo));
*/

    createAction("open_in_matrix", SLOT(slotOpenInMatrix()));
    createAction("open_in_percussion_matrix", SLOT(slotOpenInPercussionMatrix()));
    createAction("open_in_notation", SLOT(slotOpenInNotation()));
    createAction("open_in_event_list", SLOT(slotOpenInEventList()));
    createAction("set_segment_start", SLOT(slotSetSegmentStartTime()));
    createAction("set_segment_duration", SLOT(slotSetSegmentDuration()));


/*!!!

    QString pixmapDir = KGlobal::dirs()->findResource("appdata", "pixmaps/");

    Q3CanvasPixmap pixmap(pixmapDir + "/toolbar/matrix.png");
    QIcon icon = QIcon(pixmap);
    QAction *qa_open_in_matrix = new QAction( "Open in Matri&x Editor", dynamic_cast<QObject*>(this) ); //### deallocate action ptr 
			qa_open_in_matrix->setIcon(icon); 
			connect( qa_open_in_matrix, SIGNAL(triggered()), this, SLOT(slotOpenInMatrix())  );

    pixmap.load(pixmapDir + "/toolbar/matrix-percussion.png");
    icon = QIcon(pixmap);
    QAction *qa_open_in_percussion_matrix = new QAction( "Open in &Percussion Matrix Editor", dynamic_cast<QObject*>(this) ); //### deallocate action ptr 
			qa_open_in_percussion_matrix->setIcon(icon); 
			connect( qa_open_in_percussion_matrix, SIGNAL(triggered()), this, SLOT(slotOpenInPercussionMatrix())  );

    pixmap.load(pixmapDir + "/toolbar/notation.png");
    icon = QIcon(pixmap);
    QAction *qa_open_in_notation = new QAction( "Open in &Notation Editor", dynamic_cast<QObject*>(this) ); //### deallocate action ptr 
			qa_open_in_notation->setIcon(icon); 
			connect( qa_open_in_notation, SIGNAL(triggered()), this, SLOT(slotOpenInNotation())  );

    pixmap.load(pixmapDir + "/toolbar/eventlist.png");
    icon = QIcon(pixmap);
    QAction *qa_open_in_event_list = new QAction( "Open in &Event List Editor", dynamic_cast<QObject*>(this) ); //### deallocate action ptr 
			qa_open_in_event_list->setIcon(icon); 
			connect( qa_open_in_event_list, SIGNAL(triggered()), this, SLOT(slotOpenInEventList())  );

    QAction* qa_set_segment_start = new QAction(  tr("Set Segment Start Time..."), dynamic_cast<QObject*>(this) );
			connect( qa_set_segment_start, SIGNAL(toggled()), dynamic_cast<QObject*>(this), SLOT(slotSetSegmentStartTime()) );
			qa_set_segment_start->setObjectName( "set_segment_start" );		//
			//qa_set_segment_start->setCheckable( true );		//
			qa_set_segment_start->setAutoRepeat( false );	//
			//qa_set_segment_start->setActionGroup( 0 );		// QActionGroup*
			//qa_set_segment_start->setChecked( false );		//
			//### FIX: deallocate QAction ptr
			

    QAction* qa_set_segment_duration = new QAction(  tr("Set Segment Duration..."), dynamic_cast<QObject*>(this) );
			connect( qa_set_segment_duration, SIGNAL(toggled()), dynamic_cast<QObject*>(this), SLOT(slotSetSegmentDuration()) );
			qa_set_segment_duration->setObjectName( "set_segment_duration" );		//
			//qa_set_segment_duration->setCheckable( true );		//
			qa_set_segment_duration->setAutoRepeat( false );	//
			//qa_set_segment_duration->setActionGroup( 0 );		// QActionGroup*
			//qa_set_segment_duration->setChecked( false );		//
			//### FIX: deallocate QAction ptr
			

    // add undo and redo to edit menu and toolbar
    CommandHistory::getInstance()->attachView(actionCollection());
*/
}

void EditViewBase::slotConfigure()
{
    ConfigureDialog *configDlg =
        new ConfigureDialog(getDocument(), this);

//     configDlg->showPage(getConfigDialogPageIndex());
	configDlg->m_tabWidget->setCurrentIndex( getConfigDialogPageIndex() );
	
    configDlg->show();
}

void EditViewBase::slotEditKeys()
{
//&&&    KKeyDialog::configure(actionCollection());
}

void EditViewBase::slotEditToolbars()
{
//&&&
//    KEditToolbar dlg(actionCollection(), getRCFileName());

//    connect(&dlg, SIGNAL(newToolbarConfig()),
//            SLOT(slotUpdateToolbars()));

//    dlg.exec();
}

void EditViewBase::slotUpdateToolbars()
{
    createGUI(getRCFileName());
    //m_viewToolBar->setChecked(!toolBar()->isHidden());
}

void
EditViewBase::slotOpenInNotation()
{

    emit openInNotation(m_segments);
}

void
EditViewBase::slotOpenInMatrix()
{
    emit openInMatrix(m_segments);
}

void
EditViewBase::slotOpenInPercussionMatrix()
{
    emit openInPercussionMatrix(m_segments);
}

void
EditViewBase::slotOpenInEventList()
{
    emit openInEventList(m_segments);
}

std::set<int> EditViewBase::m_viewNumberPool;

std::string
EditViewBase::makeViewLocalPropertyPrefix()
{
    static char buffer[100];
    int i = 0;
    while (m_viewNumberPool.find(i) != m_viewNumberPool.end())
        ++i;
    m_viewNumber = i;
    m_viewNumberPool.insert(i);
    sprintf(buffer, "View%d::", i);
    return buffer;
}

void EditViewBase::paintEvent(QPaintEvent* e)
{
    // It is possible for this function to be called re-entrantly,
    // because a re-layout procedure may deliberately ask the event
    // loop to process some more events so as to keep the GUI looking
    // responsive.  If that happens, we remember the events that came
    // in in the middle of one paintEvent call and process their union
    // again at the end of the call.
    /*
        if (m_inPaintEvent) {
    	NOTATION_DEBUG << "EditViewBase::paintEvent: in paint event already" << endl;
    	if (e) {
    	    if (m_havePendingPaintEvent) {
    		if (m_pendingPaintEvent) {
    		    QRect r = m_pendingPaintEvent->rect().unite(e->rect());
    		    *m_pendingPaintEvent = QPaintEvent(r);
    		} else {
    		    m_pendingPaintEvent = new QPaintEvent(*e);
    		}
    	    } else {
    		m_pendingPaintEvent = new QPaintEvent(*e);
    	    }
    	}
    	m_havePendingPaintEvent = true;
    	return;
        }
    */ 
    //!!!    m_inPaintEvent = true;

    if (isCompositionModified()) {

        // Check if one of the segments we display has been removed
        // from the composition.
        //
        // For the moment we'll have to close the view if any of the
        // segments we handle has been deleted.

        for (unsigned int i = 0; i < m_segments.size(); ++i) {

            if (!m_segments[i]->getComposition()) {
                // oops, I think we've been deleted
                close();
                return ;
            }
        }
    }


    m_needUpdate = false;

    // Scan all segments and check if they've been modified.
    //
    // If we have more than one segment modified, we need to update
    // them all at once with the same time range, otherwise we can run
    // into problems when the layout of one depends on the others.  So
    // we use updateStart/End to calculate a bounding range for all
    // modifications.

    timeT updateStart = 0, updateEnd = 0;
    int segmentsToUpdate = 0;
    Segment *singleSegment = 0;

    for (unsigned int i = 0; i < m_segments.size(); ++i) {

        Segment* segment = m_segments[i];
        unsigned int refreshStatusId = m_segmentsRefreshStatusIds[i];
        SegmentRefreshStatus &refreshStatus =
            segment->getRefreshStatus(refreshStatusId);

        if (refreshStatus.needsRefresh() && isCompositionModified()) {

            // if composition is also modified, relayout everything
            refreshSegment(0);
            segmentsToUpdate = 0;
            break;

        } else if (m_timeSigNotifier->hasTimeSigChanged()) {

            // not exactly optimal!
            refreshSegment(0);
            segmentsToUpdate = 0;
            m_timeSigNotifier->reset();
            break;

        } else if (refreshStatus.needsRefresh()) {

            timeT startTime = refreshStatus.from(),
                              endTime = refreshStatus.to();

            if (segmentsToUpdate == 0 || startTime < updateStart) {
                updateStart = startTime;
            }
            if (segmentsToUpdate == 0 || endTime > updateEnd) {
                updateEnd = endTime;
            }
            singleSegment = segment;
            ++segmentsToUpdate;

            refreshStatus.setNeedsRefresh(false);
            m_needUpdate = true;
        }
    }

    if (segmentsToUpdate > 1) {
        refreshSegment(0, updateStart, updateEnd);
    } else if (segmentsToUpdate > 0) {
        refreshSegment(singleSegment, updateStart, updateEnd);
    }

    if (e) QMainWindow::paintEvent(e);

    // moved this to the end of the method so that things called
    // from this method can still test whether the composition had
    // been modified (it's sometimes useful to know whether e.g.
    // any time signatures have changed)
    setCompositionModified(false);

    //!!!    m_inPaintEvent = false;
    /*
        if (m_havePendingPaintEvent) {
    	e = m_pendingPaintEvent;
    	m_havePendingPaintEvent = false;
    	m_pendingPaintEvent = 0;
    	paintEvent(e);
    	delete e;
        }
    */
}

void EditViewBase::closeEvent(QCloseEvent* e)
{
    RG_DEBUG << "EditViewBase::closeEvent()\n";

    if (isInCtor()) {
        RG_DEBUG << "EditViewBase::closeEvent() : is in ctor, ignoring close event\n";
        e->ignore();
    } else {
//         KMainWindow::closeEvent(e);
		close(e);
    }
}

void EditViewBase::addCommandToHistory(Command *command)
{
    CommandHistory::getInstance()->addCommand(command);
}

void EditViewBase::setTool(EditTool* tool)
{
    if (m_tool)
        m_tool->stow();

    m_tool = tool;

    if (m_tool)
        m_tool->ready();

}

void EditViewBase::slotCloseWindow()
{
    close();
}
/*
void EditViewBase::slotToggleToolBar()
{
    KTmpStatusMsg msg(tr("Toggle the toolbar..."), this);

    if (m_toolBar->isVisible())
		m_toolBar->hide();
    else
		m_toolBar->show();
}
*/
void EditViewBase::slotToggleStatusBar()
{
    KTmpStatusMsg msg(tr("Toggle the statusbar..."), this);

    if (statusBar()->isVisible())
        statusBar()->hide();
    else
        statusBar()->show();
}

void EditViewBase::slotStatusMsg(const QString &text)
{
    ///////////////////////////////////////////////////////////////////
    // change status message permanently
    statusBar()->clear();
    statusBar()->showMessage(text);	//, ID_STATUS_MSG);
}

void EditViewBase::slotStatusHelpMsg(const QString &text)
{
    ///////////////////////////////////////////////////////////////////
    // change status message of whole statusbar temporary (text, msec)
    statusBar()->message(text, 2000);
}

void EditViewBase::initSegmentRefreshStatusIds()
{
    for (unsigned int i = 0; i < m_segments.size(); ++i) {

        unsigned int rid = m_segments[i]->getNewRefreshStatusId();
        m_segmentsRefreshStatusIds.push_back(rid);
    }
}

bool EditViewBase::isCompositionModified()
{
    return getDocument()->getComposition().getRefreshStatus
           (m_compositionRefreshStatusId).needsRefresh();
}

void EditViewBase::setCompositionModified(bool c)
{
    getDocument()->getComposition().getRefreshStatus
    (m_compositionRefreshStatusId).setNeedsRefresh(c);
}

bool EditViewBase::getSegmentsOnlyRestsAndClefs()
{
    using Rosegarden::Segment;

    for (unsigned int i = 0; i < m_segments.size(); ++i) {

        Segment* segment = m_segments[i];

        for (Segment::iterator iter = segment->begin();
                iter != segment->end(); ++iter) {

            if (((*iter)->getType() != Note::EventRestType)
                    && ((*iter)->getType() != Clef::EventType))
                return false;
        }

    }

    return true;

}

void EditViewBase::toggleWidget(QWidget* widget,
                                const QString& toggleActionName)
{
    QAction *toggleAction = findAction(toggleActionName);

    if (!toggleAction) {
        RG_DEBUG << "!!! Unknown toggle action : " << toggleActionName << endl;
        return ;
    }

    widget->setShown(toggleAction->isChecked());
}

void
EditViewBase::slotTestClipboard()
{
    if (getDocument()->getClipboard()->isEmpty()) {
        RG_DEBUG << "EditViewBase::slotTestClipboard(): empty" << endl;
        leaveActionState("have_clipboard");
	leaveActionState("have_clipboard_single_segment");
    } else {
        RG_DEBUG << "EditViewBase::slotTestClipboard(): not empty" << endl;
        enterActionState("have_clipboard");
        if (getDocument()->getClipboard()->isSingleSegment()) {
            enterActionState("have_clipboard_single_segment");
        } else {
            leaveActionState("have_clipboard_single_segment");
        }           
    }
}

void
EditViewBase::slotToggleSolo()
{
    QAction *toggleSoloAction = findAction("toggle_solo");
    if (!toggleSoloAction) return;

    bool newSoloState = toggleSoloAction->isChecked();

    RG_DEBUG << "EditViewBase::slotToggleSolo() : solo  = " << newSoloState << endl;
    emit toggleSolo(newSoloState);

    if (newSoloState) {
        emit selectTrack(getCurrentSegment()->getTrack());
    }
}

void
EditViewBase::slotStateChanged(const QString& s,
                               bool noReverse)
{
    RG_DEBUG << "EditViewBase::slotStateChanged " << s << ", " << noReverse << endl;
    if (noReverse) {
        enterActionState(s);
    } else {
        leaveActionState(s);
    }
}

void
EditViewBase::slotSetSegmentStartTime()
{
    Segment *s = getCurrentSegment();
    if (!s)
        return ;

    TimeDialog dialog(this, tr("Segment Start Time"),
                      &getDocument()->getComposition(),
                      s->getStartTime(), false);

    if (dialog.exec() == QDialog::Accepted) {

        SegmentReconfigureCommand *command =
            new SegmentReconfigureCommand(tr("Set Segment Start Time"));

        command->addSegment
        (s, dialog.getTime(),
         s->getEndMarkerTime() - s->getStartTime() + dialog.getTime(),
         s->getTrack());

        addCommandToHistory(command);
    }
}

void
EditViewBase::slotSetSegmentDuration()
{
    Segment *s = getCurrentSegment();
    if (!s)
        return ;

    TimeDialog dialog(this, tr("Segment Duration"),
                      &getDocument()->getComposition(),
                      s->getStartTime(),
                      s->getEndMarkerTime() - s->getStartTime(), false);

    if (dialog.exec() == QDialog::Accepted) {

        SegmentReconfigureCommand *command =
            new SegmentReconfigureCommand(tr("Set Segment Duration"));

        command->addSegment
        (s, s->getStartTime(),
         s->getStartTime() + dialog.getTime(),
         s->getTrack());

        addCommandToHistory(command);
    }
}

void EditViewBase::slotCompositionStateUpdate()
{
    // update state of 'solo' toggle
    //
    QAction *toggleSolo = findAction("toggle_solo");
    if (!toggleSolo) return;

    if (getDocument()->getComposition().isSolo()) {
        bool s = m_segments[0]->getTrack() == getDocument()->getComposition().getSelectedTrack();
        RG_DEBUG << "EditViewBase::slotCompositionStateUpdate() : set solo to " << s << endl;
        toggleSolo->setChecked(s);
    } else {
        toggleSolo->setChecked(false);
        RG_DEBUG << "EditViewBase::slotCompositionStateUpdate() : set solo to false\n";
    }

    // update the window caption
    //
    updateViewCaption();
}

void
EditViewBase::windowActivationChange(bool oldState)
{
    if (isActiveWindow()) {
        emit windowActivated();
    }
}

void
EditViewBase::handleEventRemoved(Event *event)
{
    if (m_tool)
        m_tool->handleEventRemoved(event);
}

}
#include "EditViewBase.moc"
