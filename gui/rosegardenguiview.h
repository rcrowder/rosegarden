/***************************************************************************
                          rosegardenguiview.h  -  description
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

#ifndef ROSEGARDENGUIVIEW_H
#define ROSEGARDENGUIVIEW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

// include files for Qt
#include <qcanvas.h>

#include <notationhlayout.h>
#include <notationvlayout.h>

#define KDEBUG_AREA 1010


class RosegardenGUIDoc;

/** The RosegardenGUIView class provides the view widget for the RosegardenGUIApp instance.	
 * The View instance inherits QWidget as a base class and represents the view object of a KTMainWindow. As RosegardenGUIView is part of the
 * docuement-view model, it needs a reference to the document object connected with it by the RosegardenGUIApp class to manipulate and display
 * the document structure provided by the RosegardenGUIDoc class.
 * 	
 * @author Source Framework Automatically Generated by KDevelop, (c) The KDevelop Team.
 * @version KDevelop version 0.4 code generation
 */
class RosegardenGUIView : public QCanvasView
{
  Q_OBJECT
  public:
    /** Constructor for the main view */
    RosegardenGUIView(QWidget *parent = 0, const char *name=0);
    /** Destructor for the main view */
    ~RosegardenGUIView();

    /**
     * returns a pointer to the document connected to the view
     * instance. Mind that this method requires a RosegardenGUIApp
     * instance as a parent widget to get to the window document
     * pointer by calling the RosegardenGUIApp::getDocument() method.
     *
     * @see RosegardenGUIApp#getDocument
     */
    RosegardenGUIDoc *getDocument() const;

    /** contains the implementation for printing functionality */
    void print(QPrinter *pPrinter);

    /// draw all elements
    virtual bool showElements(EventList::iterator from,
                              EventList::iterator to);

    /// same, with dx,dy offset
    virtual bool showElements(EventList::iterator from,
                              EventList::iterator to,
                              double dxoffset, double dyoffset);

    /// same, relative to the specified item
    virtual bool showElements(EventList::iterator from,
                              EventList::iterator to,
                              QCanvasItem*);

    /// Normally calls applyHorizontalLayout() then applyVerticalLayout()
    virtual bool applyLayout();

    /// Set the 'y'-coord on all doc elements - should be called before applyVerticalLayout()
    virtual bool applyHorizontalLayout();

    /// Set the 'x'-coord on all doc elements - should be called after applyHorizontalLayout()
    virtual bool applyVerticalLayout();
    
    void setHorizontalLayoutEngine(NotationHLayout* e) { m_hlayout = e; }
    void setVerticalLayoutEngine(NotationVLayout* e)   { m_vlayout = e; }

    LayoutEngine* getHorizontalLayoutEngine() { return m_hlayout; }
    LayoutEngine* getVerticalLayoutEngine()   { return m_vlayout; }

	
protected:
    /** Callback for a mouse button press event in the canvas */
    virtual void contentsMousePressEvent (QMouseEvent *e);
    /** Callback for a mouse button release event in the canvas */
    virtual void contentsMouseReleaseEvent (QMouseEvent *e);
    /** Callback for a mouse move event in the canvas */
    virtual void contentsMouseMoveEvent (QMouseEvent *e);

    void perfTest();
    void test();

private:
    QCanvasItem *m_movingItem;
    bool m_draggingItem;

    NotationHLayout* m_hlayout;
    NotationVLayout* m_vlayout;
	
};

#endif // ROSEGARDENGUIVIEW_H
