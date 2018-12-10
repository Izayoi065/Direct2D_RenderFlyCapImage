#define Def_MaxCosRange 20
#define Pixel_ex 4

#include "HandAnalyzer.h"

HandAnalyzer::HandAnalyzer()
{
	FILE *fp;
	fopen_s(&fp, "Table.dat", "rb");

	if (fp) {
		fread(pu1_RGB2Likelihood, sizeof(pu1_RGB2Likelihood[0]), 256 * 256 * 256, fp);
		fread(pf4_RGB2Likelihood, sizeof(pf4_RGB2Likelihood[0]), 256 * 256 * 256, fp);
		fclose(fp);
	}
	else {
		ResetHand();
	}
	HSV_MMPalam.flag = FALSE;
}

HandAnalyzer::~HandAnalyzer()
{
	FILE *fp;
	fopen_s(&fp, "Table.dat", "wb");

	fwrite(pu1_RGB2Likelihood, sizeof(pu1_RGB2Likelihood[0]), 256 * 256 * 256, fp);
	fwrite(pf4_RGB2Likelihood, sizeof(pf4_RGB2Likelihood[0]), 256 * 256 * 256, fp);

	fclose(fp);
}

void HandAnalyzer::ResetHand()
{
	HSV_MMPalam.flag = FALSE;
	HSV_MMPalam.MAX_Hue = 0.0f;
	HSV_MMPalam.MIN_Hue = 1.0f;
	memset(pu1_RGB2Likelihood, 0, sizeof(unsigned char) * 256 * 256 * 256);
	memset(pf4_RGB2Likelihood, 0, sizeof(float) * 256 * 256 * 256);
}

void HandAnalyzer::handLikelihood_Sampling_RGB(cv::InputArray inImage_, cv::Point tCenter, int tRadius, float * tHandLikelihood)
{
	cv::Mat inImage = inImage_.getMat();
#pragma omp parallel for
	for (int i4_tIndX = tCenter.x - tRadius; i4_tIndX <= tCenter.x + tRadius; i4_tIndX++) {
		cv::Vec3b *src = inImage.ptr<cv::Vec3b>(i4_tIndX);
		for (int i4_tIndY = tCenter.y - tRadius; i4_tIndY <= tCenter.y + tRadius; i4_tIndY++) {
			cv::Vec3b bgr = src[i4_tIndY];
			int pointBGR = i4_tIndX * size + i4_tIndY;
			if (0 < i4_tIndX && i4_tIndX < size && 0 < i4_tIndY && i4_tIndY < size &&
				(i4_tIndX - tCenter.x)*(i4_tIndX - tCenter.x) + (i4_tIndY - tCenter.y)*(i4_tIndY - tCenter.y) < tRadius*tRadius) {
				pu1_RGB2Likelihood[bgr[2] * 256 * 256 + bgr[1] * 256 + bgr[0]]++;
			}
		}
	}

	memset(pf4_RGB2Likelihood, 0, sizeof(pf4_RGB2Likelihood[0]) * 256 * 256 * 256);

#pragma omp parallel for
	for (int i4_tIndSmpR0 = 0; i4_tIndSmpR0 < 256; i4_tIndSmpR0++)
	{
		for (int i4_tIndG0 = 0; i4_tIndG0 < 256; i4_tIndG0++)
		{
			for (int i4_tIndB0 = 0; i4_tIndB0 < 256; i4_tIndB0++)
			{
				int i4_tSample = pu1_RGB2Likelihood[i4_tIndSmpR0 * 256 * 256 + i4_tIndG0 * 256 + i4_tIndB0];

				if (i4_tSample)
				{
					float f4_tRange = Def_MaxCosRange;
					float f4_tRange2 = Def_MaxCosRange * 2;
					int i4_tRange = (int)f4_tRange;

					int i4_tMinR = i4_tIndSmpR0 - i4_tRange;	if (i4_tMinR < 0)i4_tMinR = 0;
					int i4_tMaxR = i4_tIndSmpR0 + i4_tRange;	if (255 < i4_tMaxR)i4_tMaxR = 255;
					int i4_tMinG = i4_tIndG0 - i4_tRange;	if (i4_tMinG < 0)i4_tMinG = 0;
					int i4_tMaxG = i4_tIndG0 + i4_tRange;	if (255 < i4_tMaxG)i4_tMaxG = 255;
					int i4_tMinB = i4_tIndB0 - i4_tRange;	if (i4_tMinB < 0)i4_tMinB = 0;
					int i4_tMaxB = i4_tIndB0 + i4_tRange;	if (255 < i4_tMaxB)i4_tMaxB = 255;

					for (int i4_tIndSmpR1 = i4_tMinR; i4_tIndSmpR1 <= i4_tMaxR; i4_tIndSmpR1++)
					{
						for (int i4_tIndG1 = i4_tMinG; i4_tIndG1 <= i4_tMaxG; i4_tIndG1++)
						{
							for (int i4_tIndB1 = i4_tMinB; i4_tIndB1 <= i4_tMaxB; i4_tIndB1++)
							{
								if (pf4_RGB2Likelihood[i4_tIndSmpR1 * 256 * 256 + i4_tIndG1 * 256 + i4_tIndB1] < 1.0f)
								{
									float f4_tLikelihood = sqrtf((float)(
										(i4_tIndSmpR0 - i4_tIndSmpR1)*(i4_tIndSmpR0 - i4_tIndSmpR1) +
										(i4_tIndG0 - i4_tIndG1)*(i4_tIndG0 - i4_tIndG1) +
										(i4_tIndB0 - i4_tIndB1)*(i4_tIndB0 - i4_tIndB1)));
									pf4_RGB2Likelihood[i4_tIndSmpR1 * 256 * 256 + i4_tIndG1 * 256 + i4_tIndB1] += (1.0f + cosf(Def_PI*f4_tLikelihood / f4_tRange2)) / 4;
									if (1.0f < pf4_RGB2Likelihood[i4_tIndSmpR1 * 256 * 256 + i4_tIndG1 * 256 + i4_tIndB1])pf4_RGB2Likelihood[i4_tIndSmpR1 * 256 * 256 + i4_tIndG1 * 256 + i4_tIndB1] = 1.0f;
								}
							}
						}
					}
				}
			}
		}
	}
}

void HandAnalyzer::handLikelihood_Sampling_HSV(float * pf4_tH_In, float * pf4_tS_In, float * pf4_tV_In, cv::Point tCenter, int tRadius)
{
	if (HSV_MMPalam.flag == FALSE) {
		HSV_MMPalam.MAX_Hue = 0.0f;
		HSV_MMPalam.MIN_Hue = 1.0f;
		HSV_MMPalam.MAX_Saturation = HSV_MMPalam.MIN_Saturation = pf4_tS_In[tCenter.y*size + tCenter.x];
		HSV_MMPalam.MAX_Value = HSV_MMPalam.MIN_Value = pf4_tV_In[tCenter.y*size + tCenter.x];
		HSV_MMPalam.flag = TRUE;
	}

	for (int i4_tIndX = tCenter.x - tRadius; i4_tIndX <= tCenter.x + tRadius; i4_tIndX++)
	{
		for (int i4_tIndY = tCenter.y - tRadius; i4_tIndY <= tCenter.y + tRadius; i4_tIndY++)
		{
			if (0 < i4_tIndX && i4_tIndX < size && 0 < i4_tIndY && i4_tIndY < size &&
				(i4_tIndX - tCenter.x)*(i4_tIndX - tCenter.x) + (i4_tIndY - tCenter.y)*(i4_tIndY - tCenter.y) < tRadius*tRadius)
			{
				//				if(HSV_MM.f4_H_Max < pf4_tH_In[i4_tIndY*Hand_ImgW + i4_tIndX])
				//					HSV_MM.f4_H_Max = pf4_tH_In[i4_tIndY*Hand_ImgW + i4_tIndX];
				//				if(HSV_MM.f4_H_Min > pf4_tH_In[i4_tIndY*Hand_ImgW + i4_tIndX])
				//					HSV_MM.f4_H_Min = pf4_tH_In[i4_tIndY*Hand_ImgW + i4_tIndX];

				//				if(HSV_MM.f4_S_Max < pf4_tS_In[i4_tIndY*Hand_ImgW + i4_tIndX])
				//					HSV_MM.f4_S_Max = pf4_tS_In[i4_tIndY*Hand_ImgW + i4_tIndX];
				if (HSV_MMPalam.MIN_Saturation > pf4_tS_In[i4_tIndY*size + i4_tIndX])
					HSV_MMPalam.MIN_Saturation = pf4_tS_In[i4_tIndY*size + i4_tIndX];

				if (HSV_MMPalam.MAX_Value < pf4_tV_In[i4_tIndY*size + i4_tIndX])
					HSV_MMPalam.MAX_Value = pf4_tV_In[i4_tIndY*size + i4_tIndX];
				//				if(HSV_MM.f4_V_Min > pf4_tV_In[i4_tIndY*Hand_ImgW + i4_tIndX])
				//					HSV_MM.f4_V_Min = pf4_tV_In[i4_tIndY*Hand_ImgW + i4_tIndX];
			}
		}
	}
	//takamori接触判定の閾値を決定するため取得した最大値と最小値をコピー
	//MAX_Value = HSV_MMPalam.MAX_Value;
	//MIN_Value = HSV_MMPalam.MIN_Value;
}
