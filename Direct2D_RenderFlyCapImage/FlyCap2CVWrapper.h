#pragma once

class FlyCap2CVWrapper {
private:
	unsigned int offX;
	unsigned int offY;
	unsigned int width;
	unsigned int height;
	FlyCapture2::PixelFormat pixelFormat;
	FlyCapture2::Mode captureMode;

protected:
	FlyCapture2::Camera flycam;
	FlyCapture2::CameraInfo flycamInfo;
	FlyCapture2::Error flycamError;
	FlyCapture2::Image flyImg, bgrImg;
	cv::Mat cvImg;

public:
	FlyCap2CVWrapper();		// �R���X�g���N�^
	~FlyCap2CVWrapper();	// �f�X�g���N�^

	cv::Mat readImage();		// Flea3�J�������摜���擾
	void setCameraParameters();	// �J�����̎擾�摜�̐ݒ�

								/* �J�����p�����[�^�̐ݒ� */
	void autoExposure(bool flag, float absValue);
	void autoWhiteBalance(bool flag, int red, int blue);
	void autoSaturation(bool flag, float absValue);
	void autoShutter(bool flag, float ms);
	void autoGain(bool flag, float dB);
	void autoFrameRate(bool flag, float fps);
	bool checkError();
};