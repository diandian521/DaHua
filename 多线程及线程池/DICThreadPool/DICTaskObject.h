#ifndef _TASKOBJECT_H_
#define _TASKOBJECT_H_

enum EThreadRoutineExitType
{
    WAITEVENTOK = 0,
    OTHERRESON = 1
};

class CDICTaskObject
{
public:
    CDICTaskObject(){};
    virtual ~CDICTaskObject(){};
public:
    virtual EThreadRoutineExitType Run() = 0;
    virtual BOOL AutoDelete() = 0;
    HANDLE m_hStopEvent;
};

#endif
