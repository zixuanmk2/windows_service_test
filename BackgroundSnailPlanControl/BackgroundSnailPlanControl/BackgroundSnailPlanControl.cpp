// BackgroundSnailPlanControl.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "SnailPlanService.h"

int _tmain(int argc, _TCHAR* argv[])
{
	SnailPlanService service(_T("BackgroundSnailPlanControl"));
	CServiceBase::Run(service);
	return 0;
}

