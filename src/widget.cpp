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
    const int RULE_SET_LENGTH = 12;
    ruleSetName = new QString[RULE_SET_LENGTH]{"Life", "Replicator",
            "Seeds", "B25/S4", "Inkspot", "34Life",
            "Diamoeba", "2x2", "HighLife", "Day&Night",
            "Morley", "Anneal"};
    ruleSetBor = new QString[RULE_SET_LENGTH]{"3", "1;3;5;7", "2", "2;5",
            "3", "3,4", "3;5;6;7;8", "3;6",
            "3;6", "3;6;7;8", "3;6;8", "4;6;7;8"};
    ruleSetSur = new QString[RULE_SET_LENGTH]{"2;3", "1;3;5;7", "", "4",
            "0;1;2;3;4;5;6;7;8", "3;4", "5;6;7;8", "1;2;5",
            "2;3", "3;4;6;7;8", "2;4;5", "3;5;6;7;8"};
    for (int i = 0; i < RULE_SET_LENGTH; i++) {
        ui->comboBox->addItem(ruleSetName[i]);
    }
    connect(timer, SIGNAL(timeout()), ui->widget, SLOT(onTimerTimeout()));
    connect(ui->pushButton, SIGNAL(clicked(bool)), this, SLOT(onPushButtonClicked()));
    connect(ui->pushButtonClear, SIGNAL(clicked(bool)), ui->widget, SLOT(onClear()));
    connect(ui->pushButtonStep, SIGNAL(clicked(bool)), ui->widget, SLOT(onTimerTimeout()));
    connect(ui->pushButtonSave,SIGNAL(clicked(bool)), ui->widget, SLOT(onSave()));
    connect(ui->pushButtonLoad, SIGNAL(clicked(bool)), ui->widget, SLOT(onLoad()));
    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onRuleChanged()));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::onPushButtonClicked() {
    if (timer->isActive()) {
        timer->stop();
        ui->pushButton->setText("Start");
        ui->lineEditSurvive->setEnabled(true);
        ui->lineEditBorn->setEnabled(true);
        ui->lineEditSpeed->setEnabled(true);
    } else {
        timer->setInterval(ui->lineEditSpeed->text().toInt());
        onRuleChanged();
        timer->start();
        ui->lineEditSurvive->setEnabled(false);
        ui->lineEditBorn->setEnabled(false);
        ui->lineEditSpeed->setEnabled(false);
        ui->pushButton->setText("Stop");
    }
}

void Widget::onRuleChanged() {
    ui->lineEditSurvive->setText(ruleSetSur[ui->comboBox->currentIndex()]);
    ui->lineEditBorn->setText(ruleSetBor[ui->comboBox->currentIndex()]);
    ui->widget->setCanvasRule(ui->lineEditSurvive->text(),
                              ui->lineEditBorn->text());
    ui->labelShowRule->setText(ui->widget->ruleShow);
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
            if (tile[i][j] && !isSurvive(neighbor)) {
                newTile[i][j] = 0;
            } else if (!tile[i][j] && isBorn(neighbor)) {
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

void Canvas::setRule(int *sur, int *bor) {
    rule.survive = sur;
    rule.born = bor;
    qDebug() << "survive";
    for (int i = 0; rule.survive[i] != -1; i++) {
        qDebug() << rule.survive[i];
    }
    qDebug() << "born";
    for (int i = 0; rule.born[i] != -1; i++) {
        qDebug() << rule.born[i];
    }
}

void Canvas::clear() {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            tile[i][j] = 0;
        }
    }
}

void Canvas::save(QDataStream &out) {
    int* boundary = findBoundary();
    out << boundary[0] << boundary[1]
        << boundary[2] << boundary[3];
    for (int i = boundary[0]; i < boundary[1]; i++) {
        for (int j = boundary[2]; j < boundary[3]; j++) {
            out << (qint8)tile[i][j];
        }
    }
    delete[] boundary;
}

void Canvas::load(QDataStream &in) {
    int boundary[4];
    in >> boundary[0] >> boundary[1]
        >> boundary[2] >> boundary[3];
    clear();
    for (int i = boundary[0]; i < boundary[1]; i++) {
        for (int j = boundary[2]; j < boundary[3]; j++) {
            qint8 temp;
            in >> temp;
            tile[i][j] = (char)temp;
        }
    }
}

int* Canvas::findBoundary() {
    int xs = 0, xt = 0, ys = 0, yt = 0;
    for (int i = 0, sum = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            sum += tile[i][j];
        }
        xs = i;
        if (sum) {
            break;
        }
    }
    for (int i = width - 1, sum = 0; i >= 0; i--) {
        for (int j = 0; j <height; j++) {
            sum += tile[i][j];
        }
        xt = i;
        if (sum) {
            xt++;
            break;
        }
    }
    for (int i = 0, sum = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            sum += tile[j][i];
        }
        ys = i;
        if (sum) {
            break;
        }
    }
    for (int i = height - 1, sum = 0; i >= 0; i--) {
        for (int j = 0; j <width; j++) {
            sum += tile[j][i];
        }
        yt = i;
        if (sum) {
            yt++;
            break;
        }
    }
    int* rt = new int[4];
    rt[0] = xs; rt[1] = xt;
    rt[2] = ys; rt[3] = yt;
    return rt;
}

bool Canvas::isSurvive(int neighbor) {
    bool flag = 0;
    for (int i = 0; rule.survive[i] != -1; i++) {
        if (neighbor == rule.survive[i]) {
            flag = 1;
            break;
        }
    }
    return flag;
}

bool Canvas::isBorn(int neighbor) {
    bool flag = 0;
    for (int i = 0; rule.born[i] != -1; i++) {
        if (neighbor == rule.born[i]) {
            flag = 1;
            break;
        }
    }
    return flag;
}

// mywidget

MyWidget::MyWidget(QWidget *parent): QWidget(parent) {
    setMouseTracking(false);
    setCellNum(20);
    canvas = new Canvas(1000, 1000);
}

void MyWidget::cellInitialize() {
    cellSide = (this->size().width() + cellInWidth) / cellInWidth;
    cellInHeight = this->size().height() / cellSide;
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

void MyWidget::setCanvasRule(const QString& surStr, const QString& borStr) {
    int* sur = new int[9];
    int* bor = new int[9];
    QStringList surList = surStr.split(';');
    for (int i = 0; i < surList.size(); i++) {
        sur[i] = surList.at(i).toInt();
    }
    sur[surList.size()] = -1;
    QStringList borList = borStr.split(';');
    for (int i = 0; i < borList.size(); i++) {
        bor[i] = borList.at(i).toInt();
    }
    bor[borList.size()] = -1;
    canvas->setRule(sur, bor);

    // show rule in label
    QString surShowStr = surList.join("");
    QString borShowStr = borList.join("");
    ruleShow = "B" + borShowStr + "/S" + surShowStr;
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
