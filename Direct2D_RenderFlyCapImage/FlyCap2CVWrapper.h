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
	FlyCap2CVWrapper();		// コンストラクタ
	~FlyCap2CVWrapper();	// デストラクタ

	cv::Mat readImage();		// Flea3カメラより画像を取得
	void setCameraParameters();	// カメラの取得画像の設定

								/* カメラパラメータの設定 */
	void autoExposure(bool flag, float absValue);
	void autoWhiteBalance(bool flag, int red, int blue);
	void autoSaturation(bool flag, float absValue);
	void autoShutter(bool flag, float ms);
	void autoGain(bool flag, float dB);
	void autoFrameRate(bool flag, float fps);
	bool checkError();
};