#include "widget.h"
#include "ui_widget.h"
#include <QPainter>
#include <QDebug>
#include <QFileDialog>
#include <QErrorMessage>

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
    connect(ui->pushButtonStep, SIGNAL(clicked(bool)), ui->widget, SLOT(onTimerTimeout()));
    connect(ui->pushButtonSave,SIGNAL(clicked(bool)), ui->widget, SLOT(onSave()));
    connect(ui->pushButtonLoad, SIGNAL(clicked(bool)), ui->widget, SLOT(onLoad()));
}

Widget::~Widget()
{
    delete ui;
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
        ui->widget->setCanvasRule(ui->lineEditDeathLow->text().toInt(),
                                  ui->lineEditDeathUp->text().toInt(),
                                  ui->lineEditBirth->text().toInt());
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

void Canvas::save(QDataStream &out) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            out << (qint8)tile[i][j];
        }
    }
}

void Canvas::load(QDataStream &in) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            qint8 temp;
            in >> temp;
            tile[i][j] = (char)temp;
        }
    }
}

// mywidget

MyWidget::MyWidget(QWidget *parent): QWidget(parent) {
    setMouseTracking(false);
    setCellNum(20);
    canvas = new Canvas(1000, 1000);
}

void MyWidget::cellInitialize() {
    cellSide = (this->size().width() + cellInWidth) / cellInWidth;
    cellInHeight = this->size().width() / cellSide;
    startX = (canvas->getWidth() - this->size().width() / cellSide) / 2;
    startY = (canvas->getHeight() - this->size().height() / cellSide) / 2;
}

void MyWidget::paintEvent(QPaintEvent *event) {
    // painter initialization. Assign device
    QPainter* painter = new QPainter();
    painter->begin(this);

    // get canvas size. Draw background
    int wWidth = this->size().width();
    int wHeight = this->size().height();
    painter->setBrush(QBrush(Qt::white, Qt::BrushStyle::SolidPattern));
    painter->drawRect(0, 0, wWidth, wHeight);

    // initialize cell nums, and draw canvas
    painter->setBrush(QBrush(Qt::black, Qt::BrushStyle::SolidPattern));
    if (cellInWidth < 200) {
        for (int i = 0; i <= cellInWidth; i++) {
            painter->drawLine(i * cellSide, 0, i * cellSide, wHeight);
        }
        for (int i = 0; i <= cellInHeight; i++) {
            painter->drawLine(0, i * cellSide, wWidth, i * cellSide);
        }
    }

    // draw cells
    for (int i = startX; i < startX + cellInWidth; i++) {
        for (int j = startY; j < startY + cellInHeight; j++) {
            if (canvas->get(i, j)) {
                painter->drawRect(cellSide * (i - startX), cellSide * (j - startY), cellSide, cellSide);
            }
        }
    }
}

void MyWidget::mouseMoveEvent(QMouseEvent *event) {
    int mouseX = event->x();
    int mouseY = event->y();
    if (!canvas->get(mouseX / cellSide + startX, mouseY / cellSide + startY)) {
        canvas->set(mouseX / cellSide + startX, mouseY / cellSide + startY);
    }
    update();
}

void MyWidget::mousePressEvent(QMouseEvent *event) {
    int mouseX = event->x();
    int mouseY = event->y();
    canvas->set(mouseX / cellSide + startX, mouseY / cellSide + startY);
    update();
}

void MyWidget::resizeEvent(QResizeEvent *event) {
    cellInitialize();
    update();
}

void MyWidget::wheelEvent(QWheelEvent *event) {
    int wheelNum = event->delta() / 60;
    int tempNum = getCellNum() * (10.0 - wheelNum) / 10.0;
    if (tempNum <= canvas->getWidth() - tempNum && tempNum >= 2) {
        setCellNum(tempNum);
        cellInitialize();
        update();
    }
}

void MyWidget::onTimerTimeout() {
    canvas->calculate();
    update();
}

void MyWidget::onClear() {
    canvas->clear();
    update();
}

void MyWidget::onSave() {
    QString saveFileName = QFileDialog::getSaveFileName(this,
        "Save current canvas", "../file", "Canvas Files (*.dat);;All Files(*)");
    QFile file(saveFileName);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);   // we will serialize the data into the file
    qint32 magicNumber = 0xAAB1CCDD;
    out << magicNumber;
    canvas->save(out);
}

void MyWidget::onLoad() {
    QString loadFileName = QFileDialog::getOpenFileName(this,
        "Load existed canvas", "../file", "Canvas Files (*.dat);;All Files(*)");
    QFile file(loadFileName);
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);
    qint32 magicNumber;
    in >> magicNumber;
    if (magicNumber != (qint32)0xAAB1CCDD) {
        QErrorMessage msgBox;
        msgBox.showMessage("Bad File Type!");
        msgBox.exec();
        return;
    } else {
        canvas->load(in);
        update();
    }
}
