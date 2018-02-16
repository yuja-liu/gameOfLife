#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QShowEvent>
#include <QPaintEvent>
#include <QMouseEvent>

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
    //MyWidget* mywidget;

protected:
    virtual void showEvent(QShowEvent* event);

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

public:
    void initialize();
    //void calculate(int x, int y, int width, int height);
    void calculate();
    void set(int x, int y);
    void setRule(int dl, int du, int b) {
        rule.deathLow = dl;
        rule.deathUp = du;
        rule.birth = b;
    }
    void clear();
};

class MyWidget : public QWidget
{
    Q_OBJECT

    friend Widget;
private:
    int cellInWidth;
    Canvas* canvas;

public:
    explicit MyWidget(QWidget *parent = 0);

private slots:
    void onTimerTimeout();
    void onClear();

protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
};

#endif // WIDGET_H
