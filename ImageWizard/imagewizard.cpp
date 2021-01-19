#include "imagewizard.h"
#include "filechooser.h"
#include "../AlgoManager/algomanager.h"

ImageWizard::ImageWizard(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);

	frames = findChild<QStackedWidget*>("frames");

	initial = new ImageInfo;
	target = new ImageInfo;
	background = new ImageInfo;

	targetChooser = new FileChooser("Select or drag an image containing the target", initial);
	backgroundChooser = new FileChooser("Select or drag a background image", background);
	targetSelector = new TargetSelector(target);

	frames->addWidget(targetChooser);
	frames->addWidget(targetSelector);
	frames->addWidget(backgroundChooser);
}

ImageWizard::~ImageWizard() {
	delete targetChooser;
	delete backgroundChooser;
	delete targetSelector;
	delete initial;
	delete target;
	delete background;
}

//Next page in UI
void ImageWizard::goNext() {

	int cur = frames->currentIndex();
	//Restrict the ability to go to the next page if certain conditions haven't been met
	if(frames->currentWidget() == targetChooser) { //target image upload page
		if(!initial->loaded) {
			return;
		}
	}
	else if(frames->currentWidget() == targetSelector) { //target selection/crop page
		if(!target->loaded) {
			return;
		}
	}
	else if(frames->currentWidget() == backgroundChooser) { //background image upload page
		if(!background->loaded) {
			return;
		}
	}

	//if we've reached this point, then we've finished uploading/interacting with pictures on our current page and continue to the next page.
	if(cur < frames->count()) {
		frames->setCurrentIndex(++cur);
		if(frames->currentWidget() == targetSelector) {
			*(target->image) = initial->image->copy();
			targetSelector->updateImage();
		}
	}
}

//Previous page in UI 
void ImageWizard::goPrev() {

	int cur = frames->currentIndex();
	if(cur > 0) {
		frames->setCurrentIndex(--cur);
	}
}