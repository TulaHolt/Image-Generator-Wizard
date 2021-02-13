#include <QMessageBox>
#include "imagewizard.h"
#include "filechooser.h"
#include "../AlgoManager/algomanager.h" 

ImageWizard::ImageWizard(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);

	frames = findChild<QStackedWidget*>("frames");

	initial = new ImageInfo;
	target = new ImageInfo;
	background = new ImageInfo;
	destination = NULL; // new path to store

	welcomePage = new WelcomePage("Welcome to Image Generator");
	targetChooser = new FileChooser("Select or drag an image containing the target", initial, "..\\ImageGallery\\Targets\\Drones");
	backgroundChooser = new FileChooser("Select or drag a background image", background, "..\\ImageGallery\\Backgrounds");
	targetSelector = new TargetSelector("Select Target", initial, target);
	selectDestination = new SelectDestination("Select Your Destination");
	processingWindow = new ProcessingWindow("Select Your Destination");

	frames->addWidget(welcomePage);
	frames->addWidget(targetChooser);
	frames->addWidget(targetSelector);
	frames->addWidget(backgroundChooser);
	frames->addWidget(selectDestination);
	frames->addWidget(processingWindow);

	btnPrev = findChild<QPushButton*>("btnPrev");
	btnNext = findChild<QPushButton*>("btnNext");
	btnPrev->setIcon(QIcon(QDir::homePath() + "/source/repos/image-generator/icons/leftArrow.png"));
	btnNext->setIcon(QIcon(QDir::homePath() + "/source/repos/image-generator/icons/rightArrow.png"));
	btnNext->setStyleSheet("border-left: 10px transparent; border-right: 10px transparent;""border-top: 3px transparent; border-bottom: 3px transparent;"); // remove edges of button
	btnPrev->setStyleSheet("border-left: 10px transparent; border-right: 10px transparent;""border-top: 3px transparent; border-bottom: 3px transparent;"); // remove edges of button
	btnNext->setIconSize(QSize(85, 32));
	btnPrev->setIconSize(QSize(85, 32)); 
	btnPrev->setCursor(QCursor(Qt::PointingHandCursor));
	btnNext->setCursor(QCursor(Qt::PointingHandCursor));
	
	// Add lighter arrow when hovering and when disabled
	QString rightHover = QDir::homePath() + "/source/repos/image-generator/icons/rightHover.png";
	QString styleSheet = "QPushButton:hover#btnNext { icon: url(\" \"); icon - size: 38px, 30px; image: url(%1); background - repeat: no - repeat;} QPushButton:hover#btnPrev { icon: url(\" \"); icon - size: 38px, 30px; image: url(%2); background - repeat: no - repeat;} QPushButton:disabled#btnNext { icon: url(\" \"); icon - size: 38px, 30px; image: url(%3); background - repeat: no - repeat; } QPushButton:disabled#btnPrev { icon: url(\" \"); icon - size: 38px, 30px; image: url(%4); background - repeat: no - repeat; }";
	QString leftHover = QDir::homePath() + "/source/repos/image-generator/icons/leftHover.png";
	QString rightDisabled = QDir::homePath() + "/source/repos/image-generator/icons/rightDisabled.png";
	QString leftDisabled = QDir::homePath() + "/source/repos/image-generator/icons/leftDisabled.png";
	//setStyleSheet(styleSheet.arg(rightHover).arg(leftHover).arg(rightDisabled).arg(leftDisabled));

	//Hides the previous button on the first page
	btnPrev->hide();
}

ImageWizard::~ImageWizard() {
	delete welcomePage;
	delete targetChooser;
	delete backgroundChooser;
	delete targetSelector;
	delete selectDestination;
	delete processingWindow;
	delete initial;
	delete target;
	delete background;
}

void ImageWizard::enableNext() {
	btnNext->setEnabled(true);
}

void ImageWizard::disableNext() {
	btnNext->setEnabled(false);
}

void ImageWizard::enablePrev() {
	btnPrev->setEnabled(true);
}

void ImageWizard::disablePrev() {
	btnPrev->setEnabled(false);
}

//Next page in UI
void ImageWizard::goNext() {
	int cur = frames->currentIndex();

	// By default, the next button is disabled. If the page is already ready, then
	// it be re-enabled. Otherwise, it must be enabled within the page itself when
	// certain actions are performed. See: FileChooser::loadImage

	WizardPage* currentPage = dynamic_cast<WizardPage*>(frames->currentWidget());

	//Restrict the ability to go to the next page if certain conditions haven't been met

	if(!currentPage->isReady())
		return;

	if(frames->currentWidget() == targetSelector) { //target selection/crop page
		AlgoManager::AlgoManager::grabCutWrapper(target->path->toStdString());		//NOTE: Needs to be changed to target->path after SC-35 is complete 
		target->image->load(*target->path);											//Update target struct for processed image written to target->path 
	}
	else if(frames->currentWidget() == backgroundChooser) { //background image upload page
		if(!background->loaded) {
			return;
		}
	}
	else if(frames->currentWidget() == selectDestination) { //background image upload page
		if(!selectDestination->isReady()) {
			return;
		}
		destination = selectDestination->getDestination();
	}
	else if(frames->currentWidget() == processingWindow) {
		AlgoManager::AlgoManager::overlayWrapper(background->path->toStdString(), target->path->toStdString());		//Send image containing target to grabCut

	}

	//if we've reached this point, then we've finished uploading/interacting with pictures on our current page and continue to the next page.
	if(cur < frames->count()) {
		frames->setCurrentIndex(++cur);
		currentPage = dynamic_cast<WizardPage*>(frames->currentWidget());
		currentPage->pageSwitched();
		if(!currentPage->isReady())
			disableNext();
		else
			enableNext();
		//Hides & shows navigation buttons depending on the current widget
		if(cur == 1) {
			btnPrev->show();
		}
		if(cur == 5) {
			btnNext->hide();
			btnPrev->hide();
		}
		if(cur == frames->count()) {
			btnNext->hide();
		}
	}
}

//Previous page in UI 
void ImageWizard::goPrev() {
	int cur = frames->currentIndex();

	QMessageBox msg;
	msg.setText("Going back will reset the current page.");
	msg.setInformativeText("Are you sure you want to go back?");
	msg.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
	msg.setDefaultButton(QMessageBox::Cancel);
	msg.setIcon(QMessageBox::Warning);
	int ret = msg.exec();

	switch(ret) {
	case QMessageBox::Yes:
		// do nothing
		break;
	case QMessageBox::Cancel:
		return;
		break;
	}

	WizardPage* currentPage = dynamic_cast<WizardPage*>(frames->currentWidget());
	currentPage->reset();

	if(cur > 0) {
		frames->setCurrentIndex(--cur);
		currentPage = dynamic_cast<WizardPage*>(frames->currentWidget());
		currentPage->pageSwitched();
		if(!currentPage->isReady())
			disableNext();
		else
			enableNext();
		//Hides & shows navigation buttons depending on the current widget
		if(cur == 0) {
			btnPrev->hide();
		}
		if(cur == frames->count() - 2) {
			btnNext->show();
		}
	}
}