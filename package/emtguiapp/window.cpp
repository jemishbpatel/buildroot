#include <QtWidgets>
#include "window.h"
#include <unistd.h>
#include <QFile>
#include <QTextStream>
#include <sys/types.h>
#include <dirent.h>


#define INIT_STATE 0
#define BROWS_STATE 1
#define LOAD_STATE 2
#define PROCESS_STATE 3

Window::Window(QWidget *parent)
    : QWidget(parent)
{
    browseButton = createButton(tr("Browse..."), SLOT(browse()));
    loadButton = createButton(tr("LoadBMP"), SLOT(loadImage()));
    startButton = createButton(tr("Process"), SLOT(process()));
    upButton = createButton(tr("+"), SLOT(increaseLines()));
    downButton = createButton(tr("-"), SLOT(decreaseLines()));
    offsetSpinBox = createSpinBox(tr("Line: "), SLOT(valueChanged(int)));
   // startButton->setCheckable(true);

    directoryComboBox = createComboBox("");
    directoryLabel = new QLabel(tr("BMP Image:"));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setSizeConstraint(QLayout::SetNoConstraint);
    mainLayout->addWidget(directoryLabel, 0, 0);
    mainLayout->addWidget(directoryComboBox, 0, 1);
    mainLayout->addWidget(browseButton, 0, 2);
    imgDisplayLabel = new QLabel("");
    mainLayout->addWidget(imgDisplayLabel,2,0,2,2,Qt::AlignCenter);
    mainLayout->addWidget(loadButton, 4, 0);
    mainLayout->addWidget(startButton, 4, 2);
    mainLayout->addWidget(offsetSpinBox, 4, 3);
    mainLayout->addWidget(upButton, 3, 3);
    mainLayout->addWidget(downButton, 5, 3);

    progressBar = new QProgressBar;
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    mainLayout->addWidget(progressBar, 5, 0, 1, 2);
    setLayout(mainLayout);

    setWindowTitle(tr("EMT GUI"));
#if !defined(Q_OS_SYMBIAN) && !defined(Q_WS_MAEMO_5) && !defined(Q_WS_SIMULATOR)
    resize(700, 300);
#endif
    initializeDevice(&appParam);
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(LinesMonitor()));
    state = INIT_STATE;
#if 1
    if ( 0 == access("/opt/.image_file", 0) ) {
	QFile file("/opt/.image_file");
	if (!file.open(QIODevice::ReadOnly)) {
		return;
	}
	QString copy = "/opt/.image.bmp";
	directoryComboBox->addItem(copy);
        directoryComboBox->setCurrentIndex(directoryComboBox->findText(copy));
	if (loadImage() < 0) {
		file.remove();
		directoryComboBox->clear();
		removeDir("/opt/offset");
		return;
	}
	directoryComboBox->clear();
	QString image = file.readLine();
	directoryComboBox->addItem(image);
        directoryComboBox->setCurrentIndex(directoryComboBox->findText(image));

	offset = 0;
	process();
	file.close();
    }
#endif
}
Window::~Window()
{
	closeDevice(&appParam);
}

void Window::browse()
{
    QString Path;

    appParam.dataProcessed = false;
    stopStateMachine( );
    if(timer->isActive())
    	 timer->stop();
    offsetSpinBox->setValue(1);
    offsetSpinBox->setMaximum(1);
    progressBar->setValue(0);

    imgDisplayLabel->clear();
    if ( directoryComboBox->count() == 0 ) {
    	Path = QDir::currentPath();
    } else {
    	Path = directoryComboBox->currentText();
    }

    QString directory = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    "/media/",
                                                    tr("Images (*.bmp)"));
    if (!directory.isEmpty()) {
	directoryComboBox->addItem(directory);
        directoryComboBox->setCurrentIndex(directoryComboBox->findText(directory));
    }

#if 1
    QFile file ("/opt/.image_file");
    removeDir("/opt/offset");
    QFile file_copi ("/opt/.image.bmp");

    file.remove();
    file_copi.remove();
#endif
    state = BROWS_STATE;
}

void Window::increaseLines()
{
 if (offsetSpinBox->maximum() == offsetSpinBox->value())
 	valueChanged(1);
 else
 	valueChanged( offsetSpinBox->value() + 1);
}
void Window::decreaseLines()
{
 if (offsetSpinBox->minimum() == offsetSpinBox->value())
 	valueChanged(offsetSpinBox->maximum());
 else
 	valueChanged( offsetSpinBox->value() - 1);
}

int Window::loadImage()
{
    int retVal = 0;

    if ((state != INIT_STATE) && (state != BROWS_STATE))
    	return 0;

    QString path = directoryComboBox->currentText();
    QByteArray byteArr = path.toLatin1();
    char *filename = byteArr.data();
    retVal = loadActualImage(filename, &inFile, &appParam);
    if(retVal < 0) {
        int ret = QMessageBox::information(this, tr("Load status"),
                               tr("BMP file load is not completed. May be wrong image\n"));
    	return -1;
    } else {
    	QImage *inputImg = new QImage(path);
    	imgDisplayLabel->setPixmap(QPixmap::fromImage(*inputImg));
    	imgDisplayLabel->setScaledContents(true);
    	imgDisplayLabel->adjustSize();
    }
    state = LOAD_STATE;
    return 0;
}
void Window::process()
{
    int retVal = 0;

    if (state != LOAD_STATE)
    	return;

    if (appParam.dataProcessed) {
	printf("process is busy\n");
	return;
    }
    retVal = processImage(&inFile, &appParam);
    if(retVal < 0) {
        int ret = QMessageBox::information(this, tr("Process status"),
                               tr("BMP file process is not completed.\n"));
    } else {
#if 1
	QFile file ("/opt/.image_file");
	if (!(file.open(QIODevice::ReadWrite | QIODevice::Text)))
           return;
        QTextStream out(&file);

        out << directoryComboBox->currentText();
        file.close();
	QFile::copy(directoryComboBox->currentText(), "/opt/.image.bmp");
#endif
	appParam.dataLines = 1;
        retVal = configureData(&appParam);
	offsetSpinBox->setMaximum(appParam.totalBytes/appParam.totalNoEquipments);
#if 1
	QDir dir("/opt/offset");
	if ( dir.exists() ) {
		DIR *c_dir;
		struct dirent *ent;
		if ((c_dir = opendir ("/opt/offset")) != NULL) {
		    while ((ent = readdir (c_dir)) != NULL) {
		        	offset = atoi(ent->d_name);
				if (offset > 0)
					break;
			  }
			  closedir (c_dir);
		}

	} else {
		QDir().mkdir("/opt/offset");
		valueChanged(1);
		QFile file ("/opt/offset/1");
		if (!(file.open(QIODevice::ReadWrite | QIODevice::Text)))
           		return;
		file.close();
	}
#endif
	timer->start(30); //30 ms to monitor line number
        appParam.dataProcessed = true;
    }
}

void Window::LinesMonitor()
{
	if (offset > 0) {
		valueChanged(offset);
		offset = 0;
	}

	if (appParam.dataLines != offsetSpinBox->value() ) {
		QString new_file = "/opt/offset/" + QString::number(appParam.dataLines);
		QString old_file = "/opt/offset/" + QString::number(offsetSpinBox->value());
		offsetSpinBox->setValue(appParam.dataLines);
		int curVal = appParam.dataLines;
		int maxVal = offsetSpinBox->maximum();
		progressBar->setValue((100 * curVal)/maxVal);
		QFile::rename(old_file, new_file);
	}
}
void Window::valueChanged(int value)
{
	int ret = 0;
	if ( appParam.dataLines != value ) {
	     ret = setDataLines(&appParam, value);
	     if (ret < 0) {
	     	offsetSpinBox->setValue(appParam.dataLines);
	     } else {
	     	appParam.dataLines = value;
	     }
	}
	return;
}
bool Window::removeDir(const QString & dirName)
{
	bool result = true;
        QDir dir(dirName);

	if (dir.exists()) {
		Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
			if (info.isDir()) {
				result = removeDir(info.absoluteFilePath());
			}
			else {
				result = QFile::remove(info.absoluteFilePath());
			}

			if (!result) {
				return result;
			}
		}
		result = QDir().rmdir(dirName);
	}
	return result;
}

void Window::clickedaction()
{
	browse();
}

QPushButton *Window::createButton(const QString &text, const char *member)
{
	QPushButton *button = new QPushButton(text);
	connect(button, SIGNAL(clicked()), this, member);
	return button;
}

QSpinBox *Window::createSpinBox(const QString &text, const char *member)
{
	QSpinBox *offsetSpinBox = new QSpinBox;
	offsetSpinBox->setSingleStep(1);
	offsetSpinBox->setPrefix(text);
	offsetSpinBox->setMinimum(1);
	offsetSpinBox->setMaximum(1);
	offsetSpinBox->setValue(1);
	connect(offsetSpinBox, SIGNAL(valueChanged(int)), this, member);
	return offsetSpinBox;
}

QComboBox *Window::createComboBox(const QString &text)
{
	QComboBox *comboBox = new QComboBox;
	comboBox->setEditable(true);
	comboBox->addItem(text);
	comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
#if defined(Q_OS_SYMBIAN) || defined(Q_WS_MAEMO_5) || defined(Q_WS_SIMULATOR)
	comboBox->setMinimumContentsLength(3);
#endif
	connect(comboBox,SIGNAL(activated(int)),this,SLOT(clickedaction()));
	return comboBox;
}
