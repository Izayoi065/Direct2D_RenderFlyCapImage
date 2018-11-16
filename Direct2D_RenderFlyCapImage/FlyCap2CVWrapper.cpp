/****************************************************************************************************
 *�^�C�g���FFlyCapture2��OpenCV�ň������߂̃��b�p�[�N���X
 *�����@�@�FFlyCapture SDK�Ŏ擾�����摜��OpenCV��cv::Mat�^�Ŏ擾����
 *		�Fhttps://www.ptgrey.com/flycapture-sdk
 *�O��LIB �FFlyCapture2	2.10.3.169�C
 *			OpenCV		3.4.0
 *
 *���쌠�@�FTomoki Kondo
 *
 *�ύX�����F2018,11,07 Wednesday
 *�@�@�@�F�V�K�o�^
 *
 ****************************************************************************************************/
#include "FlyCap2CVWrapper.h"

FlyCap2CVWrapper::FlyCap2CVWrapper()
{
	// �J�����֐ڑ�
	flycamError = flycam.Connect(0);
	if (flycamError != FlyCapture2::PGRERROR_OK) {
		std::cout << "Failed to connect to camera" << std::endl;
		return;
	}

	//�J�������̎擾�ƕ\��
	flycamError = flycam.GetCameraInfo(&flycamInfo);
	if (flycamError != FlyCapture2::PGRERROR_OK) {
		std::cout << "Failed to get camera info from camera" << std::endl;
		return;
	}
	std::cout << flycamInfo.vendorName << " "
		<< flycamInfo.modelName << " "
		<< flycamInfo.serialNumber << std::endl;

	// Set Video Property
	// Video Mode: Custom(Format 7)
	// Frame Rate: 120fps
	flycamError = flycam.SetVideoModeAndFrameRate(FlyCapture2::VIDEOMODE_FORMAT7, FlyCapture2::FRAMERATE_FORMAT7);
	FlyCapture2::Format7ImageSettings imgSettings;
	setCameraParameters();
	imgSettings.offsetX = this->offX;
	imgSettings.offsetY = this->offY;
	imgSettings.width = this->width;
	imgSettings.height = this->height;
	imgSettings.pixelFormat = this->pixelFormat;
	imgSettings.mode = this->captureMode;
	flycamError = flycam.SetFormat7Configuration(&imgSettings, 100.0f);
	if (flycamError != FlyCapture2::PGRERROR_OK) {
		std::cout << "Failed to set video mode and frame rate" << std::endl;
		return;
	}

	// �����ύX�̖�����
	autoFrameRate(false, 60.0f);
	autoWhiteBalance(false, 640, 640);
	autoExposure(false, 1.585f);
	autoSaturation(false, 100.0f);
	autoShutter(false, 7.5f);
	autoGain(false, 0.0f);

	flycamError = flycam.StartCapture();
	if (flycamError == FlyCapture2::PGRERROR_ISOCH_BANDWIDTH_EXCEEDED) {
		std::cout << "Bandwidth exceeded" << std::endl;
		return;
	}
	else if (flycamError != FlyCapture2::PGRERROR_OK) {
		std::cout << "Failed to start image capture" << std::endl;
		return;
	}
}

FlyCap2CVWrapper::~FlyCap2CVWrapper()
{
	flycamError = flycam.StopCapture();
	if (flycamError != FlyCapture2::PGRERROR_OK) {
		// This may fail when the camera was removed, so don't show 
		// an error message
	}
	std::cout << "Disconnected to the "
		<< flycamInfo.vendorName << " " << flycamInfo.modelName
		<< " " << flycamInfo.serialNumber << std::endl;
	flycam.Disconnect();
}

// �J�����p�����[�^�̕ϐ���ݒ�
void FlyCap2CVWrapper::setCameraParameters() {
	this->offX = 160;
	this->offY = 20;
	this->width = 1008;
	this->height = 1008;
	this->pixelFormat = FlyCapture2::PIXEL_FORMAT_422YUV8;
	this->captureMode = FlyCapture2::MODE_0;
}

// �����I�o�ݒ�
// true -> auto, false -> manual
void FlyCap2CVWrapper::autoExposure(bool flag, float absValue)
{
	FlyCapture2::Property prop;
	prop.type = FlyCapture2::AUTO_EXPOSURE;
	prop.onOff = true;
	prop.autoManualMode = flag;
	prop.absControl = true;
	prop.absValue = absValue;
	flycamError = flycam.SetProperty(&prop);
	if (flycamError != FlyCapture2::PGRERROR_OK)
	{
		std::cout << "Failed to change Auto Exposure Settings" << std::endl;
	}
	return;
}

// �����z���C�g�o�����X�ݒ�
void FlyCap2CVWrapper::autoWhiteBalance(bool flag, int red, int blue)
{
	FlyCapture2::Property prop;
	prop.type = FlyCapture2::WHITE_BALANCE;
	prop.onOff = true;
	prop.autoManualMode = flag;
	prop.valueA = red;
	prop.valueB = blue;
	flycamError = flycam.SetProperty(&prop);
	if (flycamError != FlyCapture2::PGRERROR_OK)
	{
		std::cout << "Failed to change Auto White Balance Settings" << std::endl;
	}
	return;
}

// ����Satulation�ݒ�
void FlyCap2CVWrapper::autoSaturation(bool flag, float percent)
{
	FlyCapture2::Property prop;
	prop.type = FlyCapture2::SATURATION;
	prop.onOff = true;
	prop.autoManualMode = flag;
	prop.absControl = true;
	prop.absValue = percent;
	flycamError = flycam.SetProperty(&prop);
	if (flycamError != FlyCapture2::PGRERROR_OK)
	{
		std::cout << "Failed to change Auto Satulation Settings" << std::endl;
	}
	return;
}

// �����V���b�^�[���x�ݒ�
void FlyCap2CVWrapper::autoShutter(bool flag, float ms)
{
	FlyCapture2::Property prop;
	prop.type = FlyCapture2::SHUTTER;
	prop.autoManualMode = flag;
	prop.absControl = true;
	prop.absValue = ms;
	flycamError = flycam.SetProperty(&prop);
	if (flycamError != FlyCapture2::PGRERROR_OK)
	{
		std::cout << "Failed to change Auto Shutter Settings" << std::endl;
	}
	return;
}

// �����Q�C���ݒ�
void FlyCap2CVWrapper::autoGain(bool flag, float gain)
{
	FlyCapture2::Property prop;
	prop.type = FlyCapture2::GAIN;
	prop.autoManualMode = flag;
	prop.absControl = true;
	prop.absValue = gain;
	flycamError = flycam.SetProperty(&prop);
	if (flycamError != FlyCapture2::PGRERROR_OK)
	{
		std::cout << "Failed to change Auto Gain Settings" << std::endl;
	}
	return;
}

// �t���[�����[�g�ݒ�
void FlyCap2CVWrapper::autoFrameRate(bool flag, float fps)
{
	FlyCapture2::Property prop;
	prop.type = FlyCapture2::FRAME_RATE;
	prop.autoManualMode = flag;
	prop.absControl = true;
	prop.absValue = fps;
	flycamError = flycam.SetProperty(&prop);
	if (flycamError != FlyCapture2::PGRERROR_OK)
	{
		std::cout << "Failed to change Frame Rate Settings" << std::endl;
	}
	return;
}

// cv::Mat�ւ̓]��
cv::Mat FlyCap2CVWrapper::readImage() {
	// �摜�̎擾
	flycamError = flycam.RetrieveBuffer(&flyImg);
	if (flycamError != FlyCapture2::PGRERROR_OK) {
		std::cout << "capture error" << std::endl;
		return cvImg;
	}

	// BGR�ւ̕ϊ�
	flyImg.Convert(FlyCapture2::PIXEL_FORMAT_BGR, &bgrImg);

	// OpenCV��Mat�`���ɕϊ�
	unsigned int rowBytes = (unsigned int)((double)bgrImg.GetReceivedDataSize() / (double)bgrImg.GetRows());
	cvImg = cv::Mat(bgrImg.GetRows(), bgrImg.GetCols(), CV_8UC3, bgrImg.GetData(), rowBytes);

	return cvImg;
}

bool FlyCap2CVWrapper::checkError() {
	return flycamError != FlyCapture2::PGRERROR_OK;
}