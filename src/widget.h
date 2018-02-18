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

public slots:
    void onPushButtonClicked();
};

class Canvas {
    friend MyWidget;
public:
    Canvas(int w, int l):width(w), height(l) {
        initialize();
        rule.deathLow = 1;
        rule.deathUp = 4;
        rule.birth = 3;
    }
    void initialize();
    void calculate();
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
    void setRule(int dl, int du, int b) {
        rule.deathLow = dl;
        rule.deathUp = du;
        rule.birth = b;
    }
    void clear();
    void save(QDataStream& out);
    void load(QDataStream& in);

private:
    char** tile;
    char** newTile;
    int width;
    int height;
    struct Rule {
        int deathLow;
        int deathUp;
        int birth;
    } rule;
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
    void setCanvasRule(int dl, int du, int b) {
        canvas->setRule(dl, du, b);
    }
    void setCellNum(int ciw) {
        cellInWidth = ciw;
    }
    int getCellNum() {
        return cellInWidth;
    }

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
