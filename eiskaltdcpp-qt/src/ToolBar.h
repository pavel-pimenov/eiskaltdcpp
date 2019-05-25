/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include <QToolBar>
#include <QTabBar>
#include <QEvent>
#include <QShowEvent>
#include <QShortcut>
#include <QList>

#include "ArenaWidget.h"

class ArenaWidget;
class MainWindow;

class ToolBar : public QToolBar
{
    Q_OBJECT

    typedef QMap<ArenaWidget*, int> WidgetMap;

public:
    ToolBar(QWidget* = nullptr);
    virtual ~ToolBar();

    virtual bool hasWidget(ArenaWidget*) const;
    void mapWidget(ArenaWidget*);

signals:
    void widgetInserted(ArenaWidget*);

protected:
    virtual bool eventFilter(QObject *, QEvent *);
    virtual void showEvent(QShowEvent *);

public Q_SLOTS:
    void nextTab();
    void prevTab();
    void initTabs();

private Q_SLOTS:
    void slotIndexChanged(int);
    void slotTabMoved(int, int);
    void slotClose(int);
    void slotContextMenu(const QPoint&);
    void slotShorcuts();
    void insertWidget(ArenaWidget *a);
    void removeWidget(ArenaWidget*);
    void updated(ArenaWidget*);
    void mapped(ArenaWidget*);
    void toggled(ArenaWidget*);
    void redraw();


private:
    ArenaWidget *findWidgetForIndex(const int);
    void rebuildIndexes(const int);

    QTabBar *tabbar;
    QList<QShortcut*> shortcuts;
    WidgetMap map;
};
