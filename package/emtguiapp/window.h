#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QDir>
#include <QFlags>
#include <QSpinBox>
#include <QKeyEvent>
#include <QProgressBar>

extern "C" {
#include "appInfo.h"
}

QT_BEGIN_NAMESPACE
class QComboBox;
class QLabel;
class QPushButton;
class QSpinBox;
class QTableWidget;
class QTableWidgetItem;
class QKeyEvent;
class QProgressBar;
QT_END_NAMESPACE

class SpinBox : public QSpinBox
{
public:
    SpinBox(QWidget *parent = 0) : QSpinBox(parent) { }
    protected:
    void keyPressEvent(QKeyEvent *event)
    {
    	event->ignore();
    }
};

class Window : public QWidget
{
    Q_OBJECT

public:
    Window(QWidget *parent = 0);
    ~Window();

private slots:
    void browse();
    int loadImage();
    void process();
    void increaseLines();
    void decreaseLines();
    void clickedaction();
    void valueChanged(int value);
    void LinesMonitor();
    bool removeDir(const QString & dirName);

private:
    QPushButton *createButton(const QString &text, const char *member);
    QComboBox *createComboBox(const QString &text = QString());
    QSpinBox *createSpinBox(const QString &text, const char *member);

    QComboBox *directoryComboBox;
    QLabel *directoryLabel;
    QPushButton *browseButton;
    QPushButton *loadButton;
    QPushButton *startButton;
    QPushButton *upButton;
    QPushButton *downButton;
    QLabel *imgDisplayLabel;
    cBmpFile inFile;
    QSpinBox *offsetSpinBox;
    QTimer *timer;
    QProgressBar *progressBar;
    applicationParameters appParam;
    int state;
    int offset;
};
#endif
