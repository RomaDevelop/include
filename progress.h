//==================================================================================================
/*
 * ### Добавление в проект:
 * INCLUDEPATH += $$PWD/../include  # добавление пути к progress.h
 *
 * ### Применение в проекте:
 * // Объявление и инициализация (перед местом, где нужно фиксировать прогресс):
 * Progress progress(countMsgs, stepCount, [](int pr){ qdbg << "выполнено" << pr*10 << "%"; });
 *
 * progress.CheckProgressF();  // В теле цикла. При достижении каждого этапа будет вызываться обработчик прогресса.
*/
//==================================================================================================
#ifndef PROGRESSH
#define PROGRESSH
//---------------------------------------------------------------------------
#include <math.h>
//---------------------------------------------------------------------------
class Progress
{
    unsigned long long countOperations;
    unsigned long countSteps;
    unsigned long step;
    unsigned long nextStep;
    unsigned long stepsDid;
    unsigned long opsDid;
    unsigned long *progressPoints {nullptr};

    ///\brief Указатель на обработчик прогресса
    void (*CBFProgress)(int currentOperation);

public:
    ///\brief Конструктор по умолчанию. Вызов Init(...);
    Progress(unsigned long long countOperations, unsigned long countSteps, void CBFunc(int curStep) = nullptr)
    {
        Init(countOperations, countSteps, CBFunc);
    }
    ///\brief Деструктор
    ~Progress() { if(progressPoints) delete progressPoints; }

    ///\brief Статическая функция для проверки прогресса
    /// Возвращает номер этапа если текущая операция на границе этапа. Иначе возращает -1
    ///\param[in] countOperations - количество операций, которые будут произведены
    ///\param[in] countSteps - количество этапов (100 для расчёта процентов)
    ///\param[in] currStep - номер текущей операции
    inline static unsigned long CheckProgress(unsigned long long countOperations, unsigned long countSteps, unsigned long currStep)
    {
        if(countOperations == currStep) return countSteps;
        int oneStep = 1;
        if(countSteps) oneStep = ceil((double)countOperations / (double)countSteps);
        if(currStep && currStep%oneStep == 0) return currStep / oneStep;
        return -1;
    }

    ///\brief Инициализация
    ///\param[in] countOperations_ - количество операций, которые будут произведены
    ///\param[in] countSteps_ - количество этапов (100 для расчёта процентов)
    ///\param[in] CBFunc - указатель на функцию, которая будет вызвана при достижении этапа
    void Init(unsigned long long countOperations_, unsigned long countSteps_, void CBFunc(int currentOperation) = nullptr)
    {
        countOperations = countOperations_;
        countSteps = countSteps_;
        CBFProgress = CBFunc;
        if(!countOperations) countOperations = 1;
        if(!countSteps) countSteps = 1;
        step = 1;
        if(countSteps) step = ceil((double)countOperations / (double)countSteps);
        nextStep = step;
        stepsDid = 0;
        opsDid = 0;

        if(progressPoints) delete progressPoints;
        progressPoints = new unsigned long [countSteps];
        for(unsigned long i=0; i<countSteps; i++)
        {
            progressPoints[i] = ceil(((double)countOperations / (double)countSteps) * (double)(i+1));
        }
    }

    ///\brief Проверка прогресса
    /// Вызывает CBFProgress и возвращает номер этапа если текущая операция на границе этапа. Иначе возращает -1
    inline unsigned long CheckProgressF()
    {
        opsDid++;
        if(opsDid >= nextStep)
        {
            //qdbg << currentOperation << nextStep << countOperations;
            stepsDid++;
            nextStep += step;

            if(nextStep > countOperations) nextStep = countOperations;
            if(opsDid == countOperations) stepsDid = countSteps;

            if(CBFProgress) CBFProgress(stepsDid);
            return stepsDid;
        }
        return -1;
    }

    ///\brief Проверка прогресса
    /// Вызывает CBFProgress и возвращает номер этапа если текущая операция на границе этапа. Иначе возращает -1
    inline unsigned long CheckProgressS(unsigned long long currentOperation)
    {
        if(currentOperation == countOperations) return countSteps;
        if(progressPoints)
            for(unsigned long i=0; i<countSteps; i++)
                if(currentOperation == progressPoints[i]) return i+1;
        return -1;
    }

    ///\brief Возвращает количество проделанных операций
    inline unsigned long GetCountOpsDid() { return opsDid; }
};
//---------------------------------------------------------------------------
#endif
