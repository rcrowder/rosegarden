// -*- c-basic-offset: 4 -*-

/*
    Rosegarden-4
    A sequencer and musical notation editor.

    This program is Copyright 2000-2003
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

#include <klocale.h>

#include <qpainter.h>
#include <qtooltip.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qframe.h>

#include "BaseProperties.h"
#include "controlruler.h"
#include "colours.h"
#include "rosestrings.h"
#include "rosedebug.h"
#include "Segment.h"
#include "RulerScale.h"
#include "velocitycolour.h"

using Rosegarden::RulerScale;
using Rosegarden::Segment;
using Rosegarden::timeT;
using Rosegarden::PropertyName;
using Rosegarden::ViewElement;

/**
 * Control Tool
 */
class ControlTool
{
public:
    virtual ~ControlTool() {};
    virtual int operator()(double x, int val) = 0;
};

class TestTool : public ControlTool
{
public:
    int operator()(double, int);
};


int TestTool::operator()(double x, int val)
{
    int res = (int)x / 10 + val;

    RG_DEBUG << "TestTool::operator() : x = " << x
             << ", val = " << val
             << ", res = " << res << endl;

    return res;
}

//
// ------------------
//

class ControlItem : public QCanvasRectangle
{
public:
    ControlItem(ControlRuler* controlRuler,
                ViewElement* el, ViewElement* nextEl = 0);

    ViewElement* getViewElement() { return m_viewElement; }

    virtual void setValue(long);

    void setWidth(int w)  { setSize(w, height()); }
    void setHeight(int h) { setSize(width(), h); }
    int getHeight()       { return size().height(); }

    virtual void draw(QPainter &painter);

    virtual void handleMouseButtonPress(QMouseEvent *e);
    virtual void handleMouseButtonRelease(QMouseEvent *e);
    virtual void handleMouseMove(QMouseEvent *e, int deltaX, int deltaY);
    virtual void handleMouseWheel(QWheelEvent *e);

    virtual void setSelected(bool yes);

    /// recompute height according to represented value prior to a canvas repaint
    virtual void updateFromValue();

    /// update value according to height after a user edit
    virtual void updateValue();

protected:

    virtual int valueToHeight(long);
    virtual long heightToValue(int);
    virtual QColor valueToColor(int);

    //--------------- Data members ---------------------------------

    long m_value;

    ControlRuler* m_controlRuler;
    ViewElement* m_viewElement;

    static const unsigned int BorderThickness;
    static const unsigned int DefaultWidth;
};

const unsigned int ControlItem::BorderThickness = 2;
const unsigned int ControlItem::DefaultWidth    = 20;

ControlItem::ControlItem(ControlRuler* ruler, ViewElement *el,
                         ViewElement *nextEl)
    : QCanvasRectangle(ruler->canvas()),
      m_controlRuler(ruler),
      m_viewElement(el)
{
    setWidth(nextEl ? int(nextEl->getLayoutX() - el->getLayoutX()) : DefaultWidth);
    setPen(QPen(Qt::black, BorderThickness));
    setBrush(Qt::blue);

    setX(el->getLayoutX());
    setY(canvas()->height() - 10);
    updateFromValue();
    RG_DEBUG << "ControlItem x = " << x() << " - y = " << y() << endl;
    show();
}

void ControlItem::setValue(long v)
{
//     std::cerr << "ControlItem::setValue(" << v << ") x = " << x() << std::endl;

    m_value = v;
}

int ControlItem::valueToHeight(long val)
{
    return -val;
}

long ControlItem::heightToValue(int h)
{
    return -h;
}

QColor ControlItem::valueToColor(int val)
{
    QColor b = Qt::blue;
    return b.light(100 + val);
}

void ControlItem::updateValue()
{
    m_viewElement->event()->set<Rosegarden::Int>(Rosegarden::BaseProperties::VELOCITY, m_value);
}

void ControlItem::updateFromValue()
{
    if (m_viewElement->event()->get<Rosegarden::Int>(Rosegarden::BaseProperties::VELOCITY, m_value)) {
        setHeight(valueToHeight(m_value));
    }
}

void ControlItem::draw(QPainter &painter)
{
    setBrush(valueToColor(m_value));

    QCanvasRectangle::draw(painter);
}

void ControlItem::handleMouseButtonPress(QMouseEvent *e)
{
}

void ControlItem::handleMouseButtonRelease(QMouseEvent *e)
{
}

void ControlItem::handleMouseMove(QMouseEvent *e, int deltaX, int deltaY)
{
    // qDebug("ControlItem::handleMouseMove(x = %g) %p", x(), this);

    m_controlRuler->applyTool(x(), deltaY);

    setHeight(getHeight() + deltaY);
    setValue(heightToValue(getHeight()));
    updateValue();
    canvas()->update();
}

void ControlItem::handleMouseWheel(QWheelEvent *e)
{
}

void ControlItem::setSelected(bool s)
{
    QCanvasItem::setSelected(s);

    if (s) setPen(QPen(Qt::red, BorderThickness));
    else setPen(QPen(Qt::black, BorderThickness));

    canvas()->update();
}


//////////////////////////////////////////////////////////////////////

/**
 * Selector tool for the ControlRuler
 *
 * Allow the user to select several ControlItems so he can change them
 * all at the same time
 */
class ControlSelector : public QObject
{
public:
    ControlSelector(ControlRuler* parent);
    virtual ~ControlSelector() {};
    
    virtual void handleMouseButtonPress(QMouseEvent *e);
    virtual void handleMouseButtonRelease(QMouseEvent *e);
    virtual void handleMouseMove(QMouseEvent *e, int deltaX, int deltaY);

    QCanvasRectangle* getSelectionRectangle() { return m_ruler->getSelectionRectangle(); }
protected:
    //--------------- Data members ---------------------------------

    ControlRuler* m_ruler;
};

ControlSelector::ControlSelector(ControlRuler* parent)
    : QObject(parent),
      m_ruler(parent)
{
}

void ControlSelector::handleMouseButtonPress(QMouseEvent *e)
{
    qDebug("ControlSelector::handleMouseButtonPress(%d, %d)", e->x(), e->y());
    
    getSelectionRectangle()->setX(e->x());
    getSelectionRectangle()->setY(e->y());
    getSelectionRectangle()->setSize(0,0);

    getSelectionRectangle()->show();
    m_ruler->canvas()->update();
}

void ControlSelector::handleMouseButtonRelease(QMouseEvent *e)
{
    qDebug("ControlSelector::handleMouseButtonRelease");
    getSelectionRectangle()->hide();
    m_ruler->canvas()->update();
}

void ControlSelector::handleMouseMove(QMouseEvent *e, int deltaX, int deltaY)
{
    int w = int(e->x() - getSelectionRectangle()->x());
    int h = int(e->y() - getSelectionRectangle()->y());
    if (w > 0) ++w; else --w;
    if (h > 0) ++h; else --h;

    getSelectionRectangle()->setSize(w, h);

    m_ruler->canvas()->update();
}

//////////////////////////////////////////////////////////////////////

using Rosegarden::ViewElementList;

ControlRuler::ControlRuler(Rosegarden::ViewElementList* viewElementList,
                           Rosegarden::RulerScale* rulerScale,
                           QScrollBar* hsb,
                           QCanvas* c, QWidget* parent,
                           const char* name, WFlags f) :
    RosegardenCanvasView(hsb, c, parent, name, f),
    m_viewElementList(viewElementList),
    m_rulerScale(rulerScale),
    m_currentItem(0),
    m_tool(0),
    m_currentX(0.0),
    m_selecting(false),
    m_selector(new ControlSelector(this)),
    m_selectionRect(new QCanvasRectangle(canvas()))
{
    m_viewElementList->addObserver(this);
    setControlTool(new TestTool);
    m_selectionRect->setPen(Qt::red);

    init();
}


ControlRuler::~ControlRuler()
{
    m_viewElementList->removeObserver(this);
}

void ControlRuler::init()
{
    ViewElementList::iterator j;

    for(ViewElementList::iterator i = m_viewElementList->begin();
        i != m_viewElementList->end(); ++i) {

        j = i; ++j;
        // also pass next element if there's one
        //
        ControlItem* controlItem = new ControlItem(this,
                                                   *i, j != m_viewElementList->end() ? *j : 0);
    }
}

void ControlRuler::elementAdded(ViewElement *el)
{
    RG_DEBUG << "ControlRuler::elementAdded()\n";
    new ControlItem(this, el);
}

void ControlRuler::elementRemoved(ViewElement *el)
{
    RG_DEBUG << "ControlRuler::elementRemoved(\n";

    QCanvasItemList allItems = canvas()->allItems();

    for (QCanvasItemList::Iterator it=allItems.begin(); it!=allItems.end(); ++it) {
        if (ControlItem *item = dynamic_cast<ControlItem*>(*it)) {
            if (item->getViewElement() == el) {
                delete item;
                break;
            }
        }
    }
}


void
ControlRuler::slotUpdate()
{
    RG_DEBUG << "ControlRuler::slotUpdate()\n";
    canvas()->update();
}

void ControlRuler::setControlTool(ControlTool* tool)
{
    if (m_tool) delete m_tool;
    m_tool = tool;
}

void ControlRuler::contentsMousePressEvent(QMouseEvent* e)
{
    QCanvasItemList l=canvas()->collisions(e->pos());

    if (l.count() == 0) { // de-select current item
        clearSelectedItems();
        m_selecting = true;
        m_selector->handleMouseButtonPress(e);
        RG_DEBUG << "ControlRuler::contentsMousePressEvent : entering selection mode\n";
        return;
    }

    for (QCanvasItemList::Iterator it=l.begin(); it!=l.end(); ++it) {

        if (ControlItem *item = dynamic_cast<ControlItem*>(*it)) {
            if (item->isSelected()) continue;

            // clear selection unless shift was pressed
            if (!(e->state() && QMouseEvent::ShiftButton)) { clearSelectedItems(); }
            m_selectedItems << item;
            item->setSelected(true);
            item->handleMouseButtonPress(e);

        }
    }

    m_lastEventPos = e->pos();
}

void ControlRuler::contentsMouseReleaseEvent(QMouseEvent* e)
{
    if (m_selecting) {
        updateSelection();
        m_selector->handleMouseButtonRelease(e);
        RG_DEBUG << "ControlRuler::contentsMousePressEvent : leaving selection mode\n";
        m_selecting = false;
        return;
    }

    if (m_currentItem)
        m_currentItem->handleMouseButtonRelease(e);

    m_lastEventPos = e->pos();
}

void ControlRuler::contentsMouseMoveEvent(QMouseEvent* e)
{
    int deltaX = e->x() - m_lastEventPos.x(),
        deltaY = e->y() - m_lastEventPos.y();
    m_lastEventPos = e->pos();

    if (m_selecting) {
        updateSelection();
        m_selector->handleMouseMove(e, deltaX, deltaY);
        return;
    }

    if (m_lastEventPos.isNull()) m_lastEventPos = e->pos();
    
    for (QCanvasItemList::Iterator it=m_selectedItems.begin(); it!=m_selectedItems.end(); ++it) {
        if (ControlItem *item = dynamic_cast<ControlItem*>(*it))
            item->handleMouseMove(e, deltaX, deltaY);
    
    }

}

void
ControlRuler::contentsWheelEvent(QWheelEvent *e)
{
    // not sure what to do yet
    QCanvasView::contentsWheelEvent(e);
}

void ControlRuler::updateSelection()
{
    clearSelectedItems();

    QCanvasItemList l=getSelectionRectangle()->collisions(true);

    for (QCanvasItemList::Iterator it=l.begin(); it!=l.end(); ++it) {

        if (ControlItem *item = dynamic_cast<ControlItem*>(*it)) {
            item->setSelected(true);
            m_selectedItems << item;
        }
    }
}

void
ControlRuler::clearSelectedItems()
{
    for (QCanvasItemList::Iterator it=m_selectedItems.begin(); it!=m_selectedItems.end(); ++it) {
        (*it)->setSelected(false);
    }
    m_selectedItems.clear();
}

void ControlRuler::clear()
{
    QCanvasItemList list = canvas()->allItems();
    QCanvasItemList::Iterator it = list.begin();
    for (; it != list.end(); ++it) {
	if (*it != m_selectionRect)
	    delete *it;
    }
}

int ControlRuler::applyTool(double x, int val)
{
    if (m_tool) return (*m_tool)(x, val);
    return val;
}

//----------------------------------------

PropertyViewRuler::PropertyViewRuler(RulerScale *rulerScale,
                                     Segment *segment,
                                     const PropertyName &property,
                                     VelocityColour *velocityColour,
                                     double xorigin,
                                     int height,
                                     QWidget *parent,
                                     const char *name) :
    QWidget(parent, name),
    m_propertyName(property),
    m_xorigin(xorigin),
    m_height(height),
    m_currentXOffset(0),
    m_width(-1),
    m_segment(segment),
    m_rulerScale(rulerScale),
    m_fontMetrics(m_boldFont),
    m_velocityColour(velocityColour)
{
    m_boldFont.setBold(true);
    m_fontMetrics = QFontMetrics(m_boldFont);

    setBackgroundColor(RosegardenGUIColours::SegmentCanvas);

    QString tip = i18n("%1 controller").arg(strtoqstr(property));
    QToolTip::add(this, tip);
}

PropertyViewRuler::~PropertyViewRuler()
{
    // nothing
}

void
PropertyViewRuler::slotScrollHoriz(int x)
{
    m_currentXOffset = int((double(-x)) / getHScaleFactor());
    repaint();
}

QSize
PropertyViewRuler::sizeHint() const
{
    double width =
       m_rulerScale->getBarPosition(m_rulerScale->getLastVisibleBar()) +
       m_rulerScale->getBarWidth(m_rulerScale->getLastVisibleBar()) +
       m_xorigin;

    QSize res(std::max(int(width), m_width), m_height);

    return res;
}

QSize
PropertyViewRuler::minimumSizeHint() const
{
    double firstBarWidth = m_rulerScale->getBarWidth(0) + m_xorigin;
    QSize res = QSize(int(firstBarWidth), m_height);
    return res;
}

void
PropertyViewRuler::paintEvent(QPaintEvent* e)
{
    QPainter paint(this);

    if (getHScaleFactor() != 1.0) paint.scale(getHScaleFactor(), 1.0);

    paint.setPen(RosegardenGUIColours::MatrixElementBorder);

    if (m_velocityColour == 0)
       paint.setBrush(RosegardenGUIColours::MatrixElementBlock);

    paint.setClipRegion(e->region());
    paint.setClipRect(e->rect().normalize());

    QRect clipRect = paint.clipRegion().boundingRect();

    timeT from = m_rulerScale->getTimeForX
       (clipRect.x() - m_currentXOffset - m_xorigin);
//     timeT   to = m_rulerScale->getTimeForX
//        (clipRect.x() + clipRect.width() - m_currentXOffset + 100 - m_xorigin);


    Segment::iterator it = m_segment->findNearestTime(from);
    //Segment::iterator it = m_segment->begin();

    for (; m_segment->isBeforeEndMarker(it); it++) {
        long value = 0;
        
        if (!(*it)->get<Rosegarden::Int>(m_propertyName, value))
            continue;
        
        int x = int(m_rulerScale->getXForTime((*it)->getAbsoluteTime()))
            + m_currentXOffset + int(m_xorigin);

        if ((x * getHScaleFactor()) > (clipRect.x() + clipRect.width())) break;

        // include fiddle factor (+2)
        int width = 
            int(m_rulerScale->getXForTime((*it)->getAbsoluteTime() +
                                          (*it)->getDuration()) + 2)
            + m_currentXOffset + int(m_xorigin) - x;

        int blockHeight = int(double(height()) * (value/127.0));

        if (m_velocityColour)
            paint.setBrush(m_velocityColour->getColour(value));
            
        paint.drawRect(x, height() - blockHeight, width, blockHeight);
    }
}


// ----------------------------- PropertyBox -------------------------------
//

PropertyBox::PropertyBox(QString label,
                       int width,
                       int height,
                       QWidget *parent,
                       const char *name):
        QWidget(parent, name),
        m_label(label),
        m_width(width),
        m_height(height)
{
}

QSize
PropertyBox::sizeHint() const
{
    return QSize(m_width, m_height);
}


QSize
PropertyBox::minimumSizeHint() const
{
    return QSize(m_width, m_height);
}

void
PropertyBox::paintEvent(QPaintEvent *e)
{
    QPainter paint(this);

    paint.setPen(RosegardenGUIColours::MatrixElementBorder);
    //paint.setBrush(RosegardenGUIColours::MatrixElementBlock);

    paint.setClipRegion(e->region());
    paint.setClipRect(e->rect().normalize());

    paint.drawRect(2, 2, m_width - 3, m_height - 3);
    paint.drawText(10, 2 * m_height / 3, m_label);
}

