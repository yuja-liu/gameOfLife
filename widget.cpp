#include "widget.h"
#include "ui_widget.h"
#include <QPainter>
#include <QDebug>

//widget

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    timer = new QTimer();
    timer->setInterval(200);
    connect(timer, SIGNAL(timeout()), ui->widget, SLOT(onTimerTimeout()));
    connect(ui->pushButton, SIGNAL(clicked(bool)), this, SLOT(onPushButtonClicked()));
    connect(ui->pushButtonClear, SIGNAL(clicked(bool)), ui->widget, SLOT(onClear()));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::showEvent(QShowEvent *event) {
    //timer->start();
}

void Widget::onPushButtonClicked() {
    if (timer->isActive()) {
        timer->stop();
        ui->pushButton->setText("Start");
        ui->lineEditDeathLow->setEnabled(true);
        ui->lineEditDeathUp->setEnabled(true);
        ui->lineEditBirth->setEnabled(true);
        ui->lineEditSpeed->setEnabled(true);
    } else {
        timer->setInterval(ui->lineEditSpeed->text().toInt());
        timer->start();
        ui->widget->canvas->setRule(
            ui->lineEditDeathLow->text().toInt(), ui->lineEditDeathUp->text().toInt(), ui->lineEditBirth->text().toInt());
        ui->pushButton->setText("Stop");
        ui->lineEditDeathLow->setEnabled(false);
        ui->lineEditDeathUp->setEnabled(false);
        ui->lineEditBirth->setEnabled(false);
        ui->lineEditSpeed->setEnabled(false);
    }
}

// canvas

void Canvas::initialize() {
    tile = new char*[width];
    for (int i = 0; i < width; i++) {
        tile[i] = new char[height];
        for (int j = 0; j < height; j++)
            tile[i][j] = 0;
    }
    newTile = new char*[width];
    for (int i = 0; i < width; i++) {
        newTile[i] = new char[height];
        for (int j = 0; j < height; j++)
            newTile[i][j] = 0;
    }
}

void Canvas::calculate() {
    // make a copy of the original tile
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++)
            newTile[i][j] = tile[i][j];
    }

    for (int i = 1; i < width - 1; i++) {
        for (int j = 1; j < height - 1; j++) {
            int neighbor = tile[i - 1][j - 1] + tile[i][j - 1] + tile[i + 1][j - 1] +
                    tile[i - 1][j] + tile[i + 1][j] +
                    tile[i - 1][j + 1] + tile[i][j + 1] + tile[i + 1][j + 1];
            if (tile[i][j] && (neighbor <= rule.deathLow || neighbor >= rule.deathUp)) {
                newTile[i][j] = 0;
            } else if (!tile[i][j] && neighbor == rule.birth) {
                newTile[i][j] = 1;
            }
        }
    }

    //copy new tile to tile
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++)
            tile[i][j] = newTile[i][j];
    }
}

void Canvas::set(int x, int y) {
    tile[x][y] = !tile[x][y];
}

void Canvas::clear() {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            tile[i][j] = 0;
        }
    }
}

// mywidget

MyWidget::MyWidget(QWidget *parent): QWidget(parent) {
    cellInWidth = 50;
    canvas = new Canvas(1000, 1000);
}

void MyWidget::paintEvent(QPaintEvent *event) {
    QPainter* painter = new QPainter();
    painter->begin(this);
    // qDebug() << "paint event called";
    int wWidth = this->size().width();
    int wHeight = this->size().height();
    painter->setBrush(QBrush(Qt::white, Qt::BrushStyle::SolidPattern));
    painter->drawRect(0, 0, wWidth, wHeight);

    int cellWidth = int(wWidth / double(cellInWidth));
    int cellInHeight = int(wHeight / cellWidth);
    painter->setBrush(QBrush(Qt::black, Qt::BrushStyle::SolidPattern));
    for (int i = 0; i <= cellInWidth; i++) {
        painter->drawLine(i * cellWidth, 0, i * cellWidth, wHeight);
    }
    for (int i = 0; i <= cellInHeight; i++) {
        painter->drawLine(0, i * cellWidth, wWidth, i * cellWidth);
    }
    int startX = (canvas->width - cellInWidth) / 2;
    int startY = (canvas->height - cellInHeight) / 2;
    for (int i = startX; i < startX + cellInWidth; i++) {
        for (int j = startY; j < startY + cellInHeight; j++) {
            if (canvas->tile[i][j]) {
                painter->drawRect(cellWidth * (i - startX), cellWidth * (j - startY), cellWidth, cellWidth);
            }
        }
    }
}

void MyWidget::mousePressEvent(QMouseEvent *event) {
    int wWidth = this->size().width();
    int wHeight = this->size().height();
    int cellWidth = int(wWidth / double(cellInWidth));
    int cellInHeight = int(wHeight / cellWidth);
    int startX = (canvas->width - cellInWidth) / 2;
    int startY = (canvas->height - cellInHeight) / 2;
    int mouseX = event->x();
    int mouseY = event->y();
    canvas->set(mouseX / cellWidth + startX, mouseY / cellWidth + startY);
    update();
}

void MyWidget::onTimerTimeout() {
    canvas->calculate();
    update();
}

void MyWidget::onClear() {
    canvas->clear();
    update();
}
