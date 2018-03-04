#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QShowEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDataStream>

namespace Ui {
class Widget;
}

class Canvas;
class MyWidget;

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private:
    Ui::Widget *ui;
    QTimer* timer;
    QString* ruleSetName;
    QString* ruleSetSur;
    QString* ruleSetBor;

private slots:
    void onPushButtonClicked();
    void onRuleChanged();
    void onLineEditChanged();
    void onComboboxChanged();
};

class Canvas {
public:
    Canvas(int w, int l):width(w), height(l) {
        initialize();
    }
    void initialize();
    void calculate();
    void calNeighborTile();
    void set(int x, int y);
    char get(int x, int y) {
        return tile[x][y];
    }
    int getWidth() {
        return width;
    }
    int getHeight() {
        return height;
    }
    void setRule (int* sur, int* bir);
    void clear();
    void save(QDataStream& out);
    void load(QDataStream& in);
    int* findBoundary();

private:
    char** tile;
    //char** newTile;
    char** neighborTile;
    char** newNeighborTile;
    int width;
    int height;
    struct Rule {
        int* survive;
        int* born;
    } rule;
    bool isSurvive(int neighbor);
    bool isBorn(int neighbor);
};

class MyWidget : public QWidget
{
    Q_OBJECT

private:
    int cellInWidth;    // number of cell in width
    int cellInHeight;
    int cellSide;  // cell side length
    int startX; // beginning cell index
    int startY;
    void cellInitialize();
    Canvas* canvas;

public:
    explicit MyWidget(QWidget *parent = 0);
    void setCanvasRule(const QString& surStr, const QString& borStr);
    void setCellNum(int ciw) {
        cellInWidth = ciw;
    }
    int getCellNum() {
        return cellInWidth;
    }
    QString ruleShow;

private slots:
    void onTimerTimeout();
    void onClear();
    void onSave();
    void onLoad();

protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void resizeEvent(QResizeEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
};

#endif // WIDGET_H
