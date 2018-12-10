#pragma once

class HandAnalyzer
{
	typedef struct HSV_MaxMinPalam {
		bool flag;
		float MAX_Hue;
		float MAX_Saturation;
		float MAX_Value;
		float MIN_Hue;
		float MIN_Saturation;
		float MIN_Value;
	}HSV_MAXMINPALAM;
private:
	static const unsigned size = 504U;	// カメラからの入力画像の1辺のサイズ
private:
	HSV_MAXMINPALAM HSV_MMPalam;
public:
	unsigned char pu1_RGB2Likelihood[256 * 256 * 256];	// 
	float pf4_RGB2Likelihood[256 * 256 * 256];	// 
public:
	HandAnalyzer();
	virtual ~HandAnalyzer();
	void ResetHand();
	void handLikelihood_Sampling_RGB(cv::InputArray inImage_, cv::Point tCenter, int tRadius, float* tHandLikelihood);
	void handLikelihood_Sampling_HSV(float *pf4_tH_In, float *pf4_tS_In, float *pf4_tV_In, cv::Point tCenter, int tRadius);
};

