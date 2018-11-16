#include "stdafx.h"
#include "FPSCounter.h"

FPSCounter::FPSCounter(unsigned int smp)
{
	/* �T���v�����̐ݒ� */
	SetSampleNum(smp);

	/* �v�����鎞�v�̑I�� */
	if (QueryPerformanceCounter(&m_Counter) != 0)
	{
		/* QueryPerformanceCounter�֐����g�p����t���O */
		m_iCounterFlag = FPSCOUNTER_QUERYPER_COUNTER;
		m_OldLongCount = m_Counter.QuadPart;	// �������̎����i�N���b�N���j���擾
		LARGE_INTEGER Freq;
		QueryPerformanceFrequency(&Freq);		// 1sec������̃N���b�N�����擾
		m_dFreq = (double)Freq.QuadPart;		// 
	}
	else
	{
		/* timeGetTime�֐����g�p����t���O */
		m_iCounterFlag = FPSCOUNTER_TIMEGETTIME;

		/* �A�v���P�[�V�����܂��̓f�o�C�X�h���C�o�̍ŏ��^�C�}����\���A�~���b�P�ʂŎw�� */
		timeBeginPeriod(1);

		/* �������̎����i�~���b�j���擾 */
		m_dwTGTOldCount = timeGetTime();
	}

	/* �v�� */
	GetFPS();
}

FPSCounter::~FPSCounter()
{
	if (m_iCounterFlag == FPSCOUNTER_TIMEGETTIME)
		timeEndPeriod(1);    // �^�C�}�[�̐��x��߂�
}

/* FPS�l���擾 */
double FPSCounter::GetFPS()
{
	double Def = GetCurDefTime();
	if (Def == 0)
	{
		/* �v�Z�ł��Ȃ��̂�0��Ԃ� */
		return 0;
	}

	return UpdateFPS(Def);
}

/* �T���v������ύX */
void FPSCounter::SetSampleNum(unsigned int smp)
{
	m_uiNum = smp;							// ���ώ��Ԃ��v�Z����T���v����
	m_dwDefTimeLst.resize(m_uiNum, 0.0);	// ���X�g�̏�����
	m_dwSumTimes = 0;						// 
}

/* ���݂̍����������~���b�P�ʂŎ擾 */
double FPSCounter::GetCurDefTime()
{
	/* �������Ԃ��v�� */
	if (m_iCounterFlag == FPSCOUNTER_QUERYPER_COUNTER)
	{
		/* QueryPerformanceCounter�֐��ɂ��v�� */
		QueryPerformanceCounter(&m_Counter);						// ���݂̎������擾���A
		LONGLONG LongDef = m_Counter.QuadPart - m_OldLongCount;		// �����J�E���g�����Z�o����B
		double dDef = (double)LongDef;								// �{���x���������_�ɕϊ�
		m_OldLongCount = m_Counter.QuadPart;						// ���݂̎�����ێ�
		return dDef * 1000 / m_dFreq;								// �������Ԃ��~���b�P�ʂŕԂ�
	}
	else
	{
		/* timeGetTime�֐��ɂ��v�� */
		DWORD CurTime = timeGetTime();
		DWORD CurDef = CurTime - m_dwTGTOldCount;		// �����J�E���g�����Z�o����
		m_dwTGTOldCount = CurTime;						// ���݂̎�����ێ�
		return CurDef;
	}
}

/* FPS���X�V */
double FPSCounter::UpdateFPS(double Def)
{
	/* �ł��Â��f�[�^���폜 */
	m_dwDefTimeLst.pop_front();

	/* �V�����f�[�^��ǉ� */
	m_dwDefTimeLst.push_back(Def);

	/* FSP�̎Z�o */
	double FPS;
	double AveDef = (m_dwSumTimes + Def) / m_uiNum;
	if (AveDef != 0)
		FPS = 1000.0 / AveDef;

	/* ���ʉ��Z�������X�V */
	m_dwSumTimes += Def - *m_dwDefTimeLst.begin();

	return FPS;
}