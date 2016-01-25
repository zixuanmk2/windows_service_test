// BackgroundSnailPlanControl.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "SnailPlanService.h"

int _tmain(int argc, _TCHAR* argv[])
{
	SnailPlanService service(_T("BackgroundSnailPlanControl"));
	CServiceBase::Run(service);
	return 0;
}

