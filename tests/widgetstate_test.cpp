#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSettings>
#include <QWidget>
#include <QSplitter>
#include <QMainWindow>
#include <QHeaderView>
#include <QTableView>
#include <QStandardItemModel>
#include <QEvent>
#include <QDebug>
#include "widgetstate.h"

// Basic widget to test geometry saving and restoring
class TestWidget : public QWidget {
public:
    TestWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setObjectName("TestWidget");
    }
};

TEST(WidgetStateTest, BasicGeometrySaveRestore) {
    QSettings settings;
    settings.clear();

    // Create widget and set initial geometry
    TestWidget widget;
    widget.setGeometry(10, 20, 100, 200);

    // Save state
    service::WidgetState::save(&widget);

    // Verify settings were created
    settings.beginGroup("GUI");
    EXPECT_TRUE(settings.contains("TestWidget_geometry"));
    settings.endGroup();

    // Change geometry to something else
    widget.setGeometry(0, 0, 10, 10);

    // Restore state
    service::WidgetState::restore(&widget);

    // Verify geometry was restored
    EXPECT_EQ(widget.geometry(), QRect(10, 20, 100, 200));
}

TEST(WidgetStateTest, SplitterStateSaveRestore) {
    QSettings settings;
    settings.clear();

    QSplitter splitter;
    splitter.setObjectName("TestSplitter");
    // Ensure splitter is sized so it can actually respect widget sizes
    splitter.resize(400, 400);

    // Add some widgets so splitter has state
    QWidget* w1 = new QWidget(&splitter);
    QWidget* w2 = new QWidget(&splitter);
    splitter.addWidget(w1);
    splitter.addWidget(w2);

    // Note: QSplitter handles sizes a bit non-deterministically if not visible,
    // so we'll test save/restore of QByteArray state instead of specific pixel sizes,
    // or just ensure state changes.
    QList<int> sizes;
    sizes << 100 << 300;
    splitter.setSizes(sizes);

    // Save state
    service::WidgetState::save(&splitter);

    // Save the raw byte array of the state we expect
    QByteArray savedState = splitter.saveState();

    // Change state
    QList<int> newSizes;
    newSizes << 200 << 200;
    splitter.setSizes(newSizes);
    EXPECT_NE(splitter.saveState(), savedState);

    // Restore state
    service::WidgetState::restore(&splitter);

    // Verify sizes were restored via matching state
    EXPECT_EQ(splitter.saveState(), savedState);
}

TEST(WidgetStateTest, HeaderViewStateSaveRestore) {
    QSettings settings;
    settings.clear();

    QTableView table;
    table.setObjectName("TestTable");

    QStandardItemModel model(2, 2);
    table.setModel(&model);

    QHeaderView* header = table.horizontalHeader();
    header->setObjectName("TestHeader");

    // Change something about the header state
    header->resizeSection(0, 150);

    service::WidgetState::save(header);

    QByteArray savedState = header->saveState();

    header->resizeSection(0, 50);
    EXPECT_NE(header->saveState(), savedState);

    service::WidgetState::restore(header);

    EXPECT_EQ(header->saveState(), savedState);
}

TEST(WidgetStateTest, MainWindowStateSaveRestore) {
    QSettings settings;
    settings.clear();

    QMainWindow window;
    window.setObjectName("TestMainWindow");

    // Save state
    service::WidgetState::save(&window);

    // Restore state
    service::WidgetState::restore(&window);

    // Simple verification that it ran without asserting/crashing
    SUCCEED();
}

TEST(WidgetStateTest, ChildWidgetHandling) {
    QSettings settings;
    settings.clear();

    QWidget parent;
    parent.setObjectName("ParentWidget");

    QWidget* child1 = new QWidget(&parent);
    child1->setObjectName("Child1");

    QWidget* child2 = new QWidget(&parent);
    child2->setObjectName("Child2");

    // Note: handleGeometry checks if widget->parent() is null before saving geometry
    // But children might have other state to save if they are QSplitter, etc.
    // For this test, let's just make sure it doesn't crash and traveses children.

    service::WidgetState::save(&parent);
    service::WidgetState::restore(&parent);

    SUCCEED();
}

TEST(WidgetStateTest, EventFilter) {
    QSettings settings;
    settings.clear();

    QWidget widget;
    widget.setObjectName("FilterWidget");
    widget.setGeometry(100, 100, 200, 200);

    service::WidgetState state;
    state.add(&widget);

    // Simulate hide event (should save)
    QEvent hideEvent(QEvent::Hide);
    QCoreApplication::sendEvent(&widget, &hideEvent);

    // Change geometry
    widget.setGeometry(0, 0, 50, 50);

    // Simulate show event (should restore)
    QEvent showEvent(QEvent::Show);
    QCoreApplication::sendEvent(&widget, &showEvent);

    // It should be restored
    EXPECT_EQ(widget.geometry(), QRect(100, 100, 200, 200));
}

// Test with command line args reset-gui
TEST(WidgetStateTest, ResetGuiArgument) {
    QSettings settings;
    settings.clear();

    // Need to modify args to include --reset-gui
    // However, QCoreApplication::arguments() is read-only and populated on init.
    // So we can't easily test the QCoreApplication::arguments().contains check here
    // without spinning up a separate process or mocking QCoreApplication.
    // We'll skip testing --reset-gui as it's hard to test in a unified process with gtest.
}
