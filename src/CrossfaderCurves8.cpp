#include "CrossfaderCurves8.h"


//-----------------------------------------------------------------------------
HRESULT VDJ_API CCrossfaderCurves8::OnLoad()
{	
	HRESULT hr;

	#if (defined(CROSSFADERCURVES8_GUI))
		hWndPlugin = NULL;
	#endif

	memset(selectText,0,sizeof(selectText));
	memset(level1,0,sizeof(level1));
	memset(level2,0,sizeof(level2));

	select=-1;

	hr = DeclareParameterCustom(&select,ID_CUSTOM_1,"Curve","CUR",sizeof(int));
	hr = DeclareParameterSwitch(&inverted,ID_SWITCH_1,"XF Hamster", "INV",FALSE); // crossfader non inversé
	hr = DeclareParameterCustom(&VP0,ID_CUSTOM_2,"VP0","VP0",sizeof(float));
	hr = DeclareParameterCustom(&C1P1,ID_CUSTOM_3,"C1P1","C1P1",sizeof(int));
	hr = DeclareParameterCustom(&V1P1,ID_CUSTOM_4,"V1P1","V1P1",sizeof(int));
        hr = DeclareParameterCustom(&C1P2,ID_CUSTOM_5,"C1P2","C1P2",sizeof(int));
	hr = DeclareParameterCustom(&V1P2,ID_CUSTOM_6,"V1P2","V1P2",sizeof(int));
	hr = DeclareParameterCustom(&C2P1,ID_CUSTOM_7,"C2P1","C2P1",sizeof(int));
	hr = DeclareParameterCustom(&V2P1,ID_CUSTOM_8,"V2P1","V2P1",sizeof(int));
        hr = DeclareParameterCustom(&C2P2,ID_CUSTOM_9,"C2P2","C2P2",sizeof(int));
	hr = DeclareParameterCustom(&V2P2,ID_CUSTOM_10,"V2P2","V2P2",sizeof(int));

	if (select<0) select=2; // initialise au mode Full
	if (VP0<0) VP0=0.0f; // initialise si non défini
	if(C1P1<0) // initialise si non défini
	{
		C1P1=50;
		V1P1=100;
		C1P2=100;
		V1P2=0;
		C2P1=0;
		V2P1=0;
		C2P2=50;
		V2P2=100;
	}

	OnParameter(ID_INIT);
	
	return S_OK;
}
//-----------------------------------------------------------------------------
HRESULT VDJ_API CCrossfaderCurves8::OnGetPluginInfo(TVdjPluginInfo8 *infos)
{
	infos->PluginName  = "CrossfaderCurves";
	infos->Author      = "DJ CEL";
	infos->Description = "Draw your own crossfader curve";
	infos->Version     = "4.2";
	infos->Flags       = 0x00;

	
	#if (defined(CROSSFADERCURVES8_GUI)) 
		memcpy(&_TAbout,infos,sizeof(TVdjPluginInfo8));
	#endif

	return S_OK;
}
//---------------------------------------------------------------------------
ULONG VDJ_API CCrossfaderCurves8::Release()
{
	#if (defined(CROSSFADERCURVES8_GUI))
		DestroyWindowGUI();
	#endif

	delete this;
	return 0;
}
//------------------------------------------------------------------------------
// User Interface
//------------------------------------------------------------------------------
HRESULT VDJ_API CCrossfaderCurves8::OnParameter(int id)
{		
	SetParamCurves(select);

	return S_OK;
}
//------------------------------------------------------------------------------
HRESULT VDJ_API CCrossfaderCurves8::OnGetParameterString(int id, char *outParam, int outParamSize) 
{
	
	return S_OK;
}
//------------------------------------------------------------------------------
HRESULT VDJ_API CCrossfaderCurves8::OnGetUserInterface(TVdjPluginInterface8 *pluginInterface)
{

#if (defined(CROSSFADERCURVES8_GUI))
	pluginInterface->Type = VDJINTERFACE_DIALOG;

	HRESULT hr;
	double qRes;
	hr = GetInfo("get hwnd",&qRes);
	if(hr!=S_OK) hWndParent=NULL;
	else hWndParent = (VDJ_WINDOW) (INT_PTR) qRes;

	Width = 600;
	Height = 500;

	#if (defined(VDJ_WIN))
		CreateWindowGUI(hInstance,hWndParent,Width, Height, &hWndPlugin);
	#endif

	pluginInterface->hWnd = hWndPlugin;
#else
	pluginInterface->Type = VDJINTERFACE_DEFAULT;
#endif
	
	return S_OK;
}
//--------------------------------------------------------------------------
float CCrossfaderCurves8::square(float x)
{
	return x*x;
}
//--------------------------------------------------------------------------
// Algorithme principal du plugin de Crossfader Behaviors:
				
// Entrée: x=crossfader [XF] (et d'éventuels points de tracé) (on part sur une base 100 pour 2 chiffres apres la virgule)
// Sortie: y1=level 1 [level1] et y2=level 2 [level2]

bool CCrossfaderCurves8::RemplirTableauLevel1(int type)
{
	int XF;
	float a, b, c, d, e, f, R_2, x, cste, value, mincste, maxcste;

	for(XF=0;XF<=100;XF++) 
	{
		switch(type)
		{
			case 1: // Linear (3 Lines with two points defined by 4 sliders + (0,100) && (100,0) )
			{
				 // y1={100     si x=0
				 //    {-a.x+b   sur x€]0,C1P1[  a>=0 & b>0
				 //    { V1P1    si  x=C1P1
				 //    {-c.x+d   sur x€]C1P1,C1P2[ c>=0 & d>=0
				 //    {V1P2     si (x=C1P2 & C1P2!=C1P1) sinon V1P1   car on privilegie le point V1P1 pour le Cut
				 //    {-e.x+f   sur ]C1P2,100[  e>=0 & f>=0
				 //    {0        si x=100
				if(XF==0) level1[XF]=100.0f;
				else if(XF>0 && XF<C1P1) 
				{
					//(0,100) && (C1P1,V1P1)
					if (C1P1 == 0) // Division par 0 ssi C1P1=0 :impossible dans cet intervalle
					{
						level1[XF] = 0.0f;
					}
					else
					{
						b = 100.0f;
						a = (float)(b - V1P1) / (float)C1P1;
						level1[XF] = -a * XF + b;
					}
				}
				else if (XF==C1P1 && C1P1!=0 && C1P1!=100) level1[XF]=float(V1P1);
				else if (XF>C1P1 && XF<C1P2) 
				{
					// (C1P1,V1P1) && (C1P2,V2P2)
					if ((C1P2 - C1P1) == 0)   // Division par 0 ssi C1P1=C1P2=2048 ("Cut"): impossible dans cet intervalle
					{
						level1[XF] = 0.0f;
					}
					else
					{
						c = (float)(V1P1 - V1P2) / (float)(C1P2 - C1P1);
						d = float(V1P1) + c * float(C1P1);
						level1[XF] = -c * XF + d;
					}
				}
				else if (XF==C1P2 && C1P2!=100 && C1P2!=0)
				{
					if (C1P2==C1P1) level1[XF]=float(V1P1);  // Gestion du "Cut"
					else level1[XF]=float(V1P2); 
				}
				else if (XF>C1P2 && XF<100)
				{	
					// (C1P2,V1P2) && (100,0)
					if (C1P2!=100)  //Division par 0 impossible dans cet intervalle
					e=(float)V1P2/(float)(100-C1P2); 
					f=-e*100.0f;
					
					level1[XF]=-e*XF+f;
				}
				else if (XF==100) level1[XF]=0.0f;
			}
			break;

			case 2: //circle haut
			{
				// equation d'un cercle : (x-a)^2 + (y-b)^2 = R^2  
				//                     <=>  y = b + sqrt(R^2 - (x-a)^2)
				a = -VP0;
				b = -VP0;
				R_2 = square(VP0 + 100.0f) + square(VP0);
				x = float(XF);
				value = R_2 - square(x - a); 
				if (value<0) level1[XF]=0;
				else level1[XF] = b + (float) sqrt(value);
			}
			break;

			case 3: //circle bas
			{
				// equation d'un cercle : (x-a)^2 + (y-b)^2 = R^2  
				//                     <=>  y = b + sqrt(R^2 - (x-a)^2)
				a = - VP0;
				b = - VP0;
				R_2 = square(VP0 + 100.0f) + square(VP0);
				x = 100.0f - float(XF);
				value=R_2 - square(x - a);
				if (value<0) level1[XF]=0;
				else level1[XF]= 100.0f - (b + (float) sqrt(value));
			}
			break;

			case 4:  //Log
			{
				x = 100.0f - float(XF);
				cste=100/(float)log10(100.0f);
				value=float(x+1);
				
				if (value<0) level1[XF]=0; 
				else level1[XF]=cste*(float)log10(value);
			}
			break;

			case 5:  // High Power
			{
				x = float(XF)/100.0f;
				mincste = 1.0f;
				maxcste = 200.0f; // puissance max
				cste = mincste + VP0/100.0f * (maxcste - mincste);
				level1[XF]=100.0f * (1.0f - (float) pow(x, cste));
			}
			break;

			case 6:  // Low Power
			{
				x = 1.0f - float(XF)/100.0f;
				mincste = 1.0f;
				maxcste = 200.0f; // puissance max
				cste = mincste + VP0/100.0f * (maxcste - mincste);
				level1[XF]=100.0f - 100.0f*(1.0f - (float) pow(x, cste));
			}
			break;

			case 7:  // S-Curve
			{
				x = float(XF)/100.0f;
				level1[XF]=100.0f * (1.0f+(float) cos(x*M_PI))/2.0f;
			}
			break;
		}

		if (level1[XF]<=0) level1[XF]=0.0f;
		if (level1[XF]>=100) level1[XF]=100.0f;

		// Conversion en float
		if(inverted==TRUE) level[1][XF] = level1[XF]/ float(100);
		else level[0][XF] = level1[XF]/ float(100);
	}

	return true;
}
//--------------------------------------------------------------------------
bool CCrossfaderCurves8::RemplirTableauLevel2(int type)
{		
	int XF;
	float a, b, g, h, i, j, k, l, R_2, x, cste, value, mincste, maxcste;
	
	for(XF=0;XF<=100;XF++) 
	{	
		switch(type)
		{
			case 1:
			{
				// Linear (3 Lines with two points defined by 4 sliders + (0,0) && (100,100))
				// y2={0       si x=0      
				//    {g.x+h   sur x€]0,C2P1[ g>=0 & h>=0
				//    {V2P1    si (x=C2P1 && C2P1!=C2P2) sinon V2P2
				//    {i.x+j  sur x€]C2P1,C2P2[   i>=0 & j>=0
				//    {V2P2  si x=C2P2
				//    {k.x+l   sur x€]C2P2,100[  k>=0 & l>0
				//    {100   si x=100
			
			
				if (XF==0) level2[XF]=0.0f;
				else if (XF>0 && XF<C2P1)
				{
					//(0,0) && (C2P1,V2P1)
					h=0;
					if (C2P1!=0)   // Division par 0 si C2P1=0 
						g=(float)V2P1/(float)C2P1;
	              
					level2[XF]=g*XF+h;
				}
				else if(XF==C2P1 && C2P1!=0 && C2P1!=100)
				{		
					if (C2P1==C2P2) level2[XF]=float(V2P2);  // Gestion du "Cut"
					else level2[XF]=float(V2P1); 
				}
				else if (XF>C2P1 && XF<C2P2)
				{
					// (C2P1,V2P1) && (C2P2,V2P2)
					if ((C2P2 - C2P1) == 0) //Division par 0 ssi C2P1=C2P2=2048 ("Cut"): impossible
					{
						level2[XF] = 0.0f;
					}   
					else
					{
						i = (float)(V2P2 - V2P1) / (float)(C2P2 - C2P1);
						j = float(V2P2) - i * float(C2P2);
						level2[XF] = i * XF + j;
					}
				}
				else if (XF==C2P2 && C2P2!=100 && C2P2!=0)
				{
					level2[XF]=float(V2P2);
				}
				else if(XF>C2P2 && XF<100)
				{
					//(C2P2,V2P2) && (100,100)
					if((100-C2P2)!=0)           // Division par 0 ssi C2P2=100 :Impossible dans cet intervalle
					k=(float)(100-V2P2)/(float)(100-C2P2);
					l=(100-100*k);
					level2[XF]=k*XF+l;
				}
				else if (XF==100) level2[XF]=100.0f;
			}
			break;

			case 2: // High circle
			{
				// equation d'un cercle : (x-a)^2 + (y-b)^2 = R^2  
				//                     <=>  y = b + sqrt(R^2 - (x-a)^2)
				a = - VP0;
				b = - VP0;
				R_2 = square(VP0 + 100.0f) + square(VP0);
				x = 100.0f - float(XF);
				value=R_2 - square(x - a);
				if (value<0) level2[XF]=0;
				else level2[XF]= b + (float) sqrt(value);
			}
			break;

			case 3://circle bas
			{
				// equation d'un cercle : (x-a)^2 + (y-b)^2 = R^2  
				//                     <=>  y = b + sqrt(R^2 - (x-a)^2)
				a = -VP0;
				b = -VP0;
				R_2 = square(VP0 + 100.0f) + square(VP0);
				x = float(XF);
				value = R_2 - square(x - a);
				if (value<0) level2[XF]=0;
				else level2[XF] = 100.0f - (b + (float) sqrt(value));
			}
			break;

			case 4:
			{
				cste=100.0f/(float)log10(100.0f);
				value=float(XF+1);
				if (value<0) level2[XF]=0; 
				else level2[XF] = cste*(float)log10(value);
			}
			break;

			case 5:  //High Power
			{
				x = 1.0f - float(XF)/100.0f;
				mincste = 1.0f;
				maxcste = 200.0f; // puissance max
				cste = mincste + VP0/100.0f * (maxcste - mincste);
				level2[XF]=100.0f * (1.0f - (float) pow(x, cste));
			}
			break;

			case 6:  //Low Power
			{
				x = float(XF)/100.0f;
				mincste = 1.0f;
				maxcste = 200.0f; // puissance max
				cste = mincste + VP0/100.0f * (maxcste - mincste);
				level2[XF]=100.0f - 100.0f * (1.0f - (float) pow(x, cste));
			}
			break;

			case 7:  //S-Curve
			{
				x = float(XF)/100.0f;
				level2[XF]=100.0f*(1-(float)cos(x*M_PI))/2.0f;
			}
			break;
		}

		if (level2[XF]<=0) level2[XF]=0.0f;
		if (level2[XF]>=100) level2[XF]=100.0f;

	    // Conversion en float
		if(inverted==TRUE) level[0][XF] = level2[XF]/ 100.0f;
		else level[1][XF] = level2[XF]/ 100.0f;
	}

	return true;
}
//--------------------------------------------------------------------------
HRESULT CCrossfaderCurves8::SetCrossfaderCurve()
{
	HRESULT hr = S_OK;
	char code[17];
	char data[1717];
	char cmdFull[1736];
	int XF = 0;

	ZeroMemory(cmdFull,sizeof(cmdFull));
	ZeroMemory(data,sizeof(data));
	ZeroMemory(code,sizeof(code));
	
	for(XF=0;XF<=100;XF++) 
	{	
		if(level[0][XF]>=0.0f && level[0][XF]<=1.0f && level[1][XF]>=0.0f && level[1][XF]<=1.0f)
		{
			sprintf(code,"%.2f=[%.2f,%.2f]",XF/float(100),level[0][XF],level[1][XF]);
			sprintf(data,"%s%s",data,code);
			if(XF<100) sprintf(data,"%s%s",data,"/");
		}
	}
	
	sprintf(cmdFull,"crossfader_curve '%s'",data);

	hr = SendCommand(cmdFull);

	return hr;
}
//----------------------------------------------------------------------------------------
//  GUI INTERFACE
//----------------------------------------------------------------------------------------
#if (defined(CROSSFADERCURVES8_GUI) && defined(VDJ_WIN))
//----------------------------------------------------------------------------------------
void CCrossfaderCurves8::CreateWindowGUI(HINSTANCE hInstance,HWND hWndParent,int Width, int Height, HWND *hWndPlugin)
{
	int DialogWidth;
	int DialogHeight;
	
	if(Width<0 || Height<0)
	{
		DialogWidth = 320;
		DialogHeight = 240;
	}
	else
	{
		DialogWidth = Width;
		DialogHeight = Height;
	}
	
	if (!IsWindow(*hWndPlugin)) 
	{
		*hWndPlugin = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), hWndParent, DialogProcStatic, (LPARAM)this);

		MoveWindow(*hWndPlugin,0,0,DialogWidth,DialogHeight,TRUE);
		
		ShowWindow(*hWndPlugin,SW_SHOW);
	}

}
//----------------------------------------------------------------------------------------
void CCrossfaderCurves8::CloseWindowGUI(HWND hDlg)
{
	SendCommand("effect_show_gui");
}
//----------------------------------------------------------------------------------------
void CCrossfaderCurves8::DestroyWindowGUI()
{
	DestroyWindow(hWndPlugin);
	hWndPlugin = NULL;
}
//----------------------------------------------------------------------------------------
INT_PTR CALLBACK CCrossfaderCurves8::DialogProcStatic(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	if (message == WM_INITDIALOG)
	{
		SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR) lParam);
	}

	LPARAM pThis = GetWindowLongPtr(hDlg, GWLP_USERDATA);
	if (!pThis) return FALSE;

	return (((CCrossfaderCurves8*)pThis)->DialogProc(hDlg, message, wParam, lParam));
}
//----------------------------------------------------------------------------------------
INT_PTR CALLBACK CCrossfaderCurves8::DialogProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;

	switch(message)
	{
		case WM_INITDIALOG:
			InitMenu();
			InitInterface(hDlg);
			break;

	    case WM_CLOSE:
			CloseWindowGUI(hDlg);
			break;
		
		case WM_DESTROY:
			ReleaseMenu();
			ReleaseInterface(hDlg);
			break;	

		case WM_SIZE:
			OnResize(hDlg,LOWORD(lParam),HIWORD(lParam));
			break;

		case WM_PAINT:
			hDC = BeginPaint(hDlg,&ps);	
			OnPaint(hDC);
			EndPaint(hDlg,&ps); 	
			break;
	
		case WM_COMMAND:
			OnCommand(hDlg,LOWORD(wParam));
			break;
			
		case WM_MOUSEMOVE:
			GetCursorPos(&pt);
			ScreenToClient(hDlg,&pt);
			OnMouseMove(hDlg, pt.x,pt.y);
			break;

		case WM_MOUSELEAVE:
			OnMouseLeave(hDlg);
			break;

		case WM_LBUTTONDOWN:
			OnMouseDown(hDlg, pt.x,pt.y,0);
			break;

		case WM_LBUTTONUP:
			OnMouseUp(hDlg, pt.x,pt.y,0);
			break;

		case WM_RBUTTONDOWN:
			OnMouseDown(hDlg, pt.x,pt.y,1);
			break;

		case WM_RBUTTONUP:
			OnMouseUp(hDlg, pt.x,pt.y,1);
			break;

		default:
			return FALSE;
	}
	return TRUE;
}
//---------------------------------------------------------------------------
void CCrossfaderCurves8::InitInterface(HWND hDlg)
{
	NB_BUTTONS = 0;
	mousedown_curves = false;
	select_level1 = true;

	if(select==1 || select==17 || select == 18 || select == 19 || select == 21) lock_custom=false;
	else lock_custom=true;
			
	button1_down=false;
	button2_down=false;
	button4_down=false;
	point1_down=false;
	point2_down=false;
	point0_down=false;
	
	xMiddle_curves_XF = 0;
	yMiddle_curves_level = 0;
	xP0 = 0;

	hBrushInterfaceCurvesBackground = CreateSolidBrush(RGB(0, 0, 0));  // black
	hBrushButton1Background         = CreateSolidBrush(RGB(35, 36, 37)); // gray
	hBrushButton2Background = CreateSolidBrush(RGB(21, 108, 137)); // gray light
	hPenButton1Border = CreatePen(PS_SOLID, 2, RGB(58, 57, 59)); // gray : up
	hPenButton2Border = CreatePen(PS_SOLID, 2, RGB(58, 57, 59)); // purple : down

	hPenInterfaceCurvesBorder = CreatePen(PS_SOLID, 1, RGB(0, 0, 0)); // black
	hPenAxes                  =	CreatePen(PS_SOLID, 3, RGB(150, 150, 150)); // gray
	hPenLevel1                = CreatePen(PS_SOLID, 3, RGB(172, 47, 55)); // red
	hPenLevel2                = CreatePen(PS_SOLID, 3, RGB(21, 108, 137)); // blue
	hPenLevel3                = CreatePen(PS_SOLID, 3, RGB(255, 251, 170)); // yellow
	hPointBorderUp            = CreatePen(PS_SOLID, 3, RGB(255, 255, 255)); // white : up
	hPointBorderDown          = CreatePen(PS_SOLID, 3, RGB(255, 0, 255));   // purple : down

	ZeroMemory(&r6,sizeof(RECT));
	ZeroMemory(&r7,sizeof(RECT));
	ZeroMemory(&r8,sizeof(RECT));
	ZeroMemory(&r9,sizeof(RECT));
}
//---------------------------------------------------------------------------
void CCrossfaderCurves8::ReleaseInterface(HWND hDlg)
{
	DeleteObject(hBrushInterfaceCurvesBackground);
	DeleteObject(hPenInterfaceCurvesBorder);
	DeleteObject(hPenAxes);
	DeleteObject(hPenButton1Border);
	DeleteObject(hBrushButton1Background);
	DeleteObject(hPenButton2Border);
	DeleteObject(hBrushButton2Background);
	DeleteObject(hPenLevel1);
	DeleteObject(hPenLevel2);
	DeleteObject(hPenLevel3);
	DeleteObject(hPointBorderUp);
	DeleteObject(hPointBorderDown);
}
//---------------------------------------------------------------------------
void CCrossfaderCurves8::InitMenu()
{
	hMenu=CreatePopupMenu();
    hSubMenu1=CreatePopupMenu();
	hSubMenu2=CreatePopupMenu();
	hSubMenu3=CreatePopupMenu();
	hSubMenu4=CreatePopupMenu();
	hSubMenu5=CreatePopupMenu();
	hSubMenu6=CreatePopupMenu();
	hSubMenu7=CreatePopupMenu();

	AppendMenu(hMenu,MF_POPUP|MF_STRING,(UINT_PTR) hSubMenu1,"Linear");
	AppendMenu(hSubMenu1,MF_STRING,IDM_SELECTION_1,"Customize");
	AppendMenu(hSubMenu1,MF_SEPARATOR,0,NULL);
	AppendMenu(hSubMenu1,MF_STRING,IDM_SELECTION_2,"Full");
	AppendMenu(hSubMenu1,MF_STRING,IDM_SELECTION_3,"Smooth");
	AppendMenu(hSubMenu1,MF_STRING,IDM_SELECTION_4,"Cut");
	AppendMenu(hSubMenu1,MF_STRING,IDM_SELECTION_5,"Scratch");
	AppendMenu(hSubMenu1,MF_STRING,IDM_SELECTION_6,"Cross");
	AppendMenu(hSubMenu1,MF_STRING,IDM_SELECTION_7,"New 2(1)");
	AppendMenu(hSubMenu1,MF_STRING,IDM_SELECTION_8,"New 2(2)");
	AppendMenu(hSubMenu1,MF_STRING,IDM_SELECTION_9,"V");
	AppendMenu(hSubMenu1,MF_STRING,IDM_SELECTION_10,"Smooth V");
	AppendMenu(hSubMenu1,MF_STRING,IDM_SELECTION_11,"Long Full");
	AppendMenu(hMenu,MF_POPUP|MF_STRING,(UINT_PTR) hSubMenu2,"Non-Linear (High Circle)");
	AppendMenu(hSubMenu2,MF_STRING,IDM_SELECTION_17,"Customize");
	AppendMenu(hSubMenu2,MF_SEPARATOR,0,NULL);
	AppendMenu(hSubMenu2,MF_STRING,IDM_SELECTION_12,"High Circle Max");
	AppendMenu(hSubMenu2,MF_STRING,IDM_SELECTION_13,"High Circle 3/4");
	AppendMenu(hSubMenu2,MF_STRING,IDM_SELECTION_14,"High Circle -3dB");
	AppendMenu(hMenu,MF_POPUP|MF_STRING,(UINT_PTR) hSubMenu3,"Non-Linear (Low Circle)");
	AppendMenu(hSubMenu3,MF_STRING,IDM_SELECTION_18,"Customize");
	AppendMenu(hSubMenu3,MF_SEPARATOR,0,NULL);
	AppendMenu(hSubMenu3,MF_STRING,IDM_SELECTION_15,"Low Circle");
	AppendMenu(hMenu,MF_POPUP|MF_STRING,(UINT_PTR) hSubMenu4,"Non-Linear (Log)");
	AppendMenu(hSubMenu4,MF_STRING,IDM_SELECTION_16,"Log");
	AppendMenu(hMenu,MF_POPUP|MF_STRING,(UINT_PTR) hSubMenu5,"Non-Linear (High Power)");
	AppendMenu(hSubMenu5,MF_STRING,IDM_SELECTION_19,"Customize");
	AppendMenu(hSubMenu5,MF_SEPARATOR,0,NULL);
	AppendMenu(hSubMenu5,MF_STRING,IDM_SELECTION_20,"High Power Medium");
	AppendMenu(hSubMenu5,MF_STRING,IDM_SELECTION_24, "High Power Very Low");
	AppendMenu(hMenu,MF_POPUP|MF_STRING,(UINT_PTR) hSubMenu6,"Non-Linear (Low Power)");
	AppendMenu(hSubMenu6,MF_STRING,IDM_SELECTION_21,"Customize");
	AppendMenu(hSubMenu6,MF_SEPARATOR,0,NULL);
	AppendMenu(hSubMenu6,MF_STRING,IDM_SELECTION_22,"Low Power Medium");
	AppendMenu(hMenu,MF_POPUP|MF_STRING,(UINT_PTR) hSubMenu7,"Non-Linear (S-Curve)");
	AppendMenu(hSubMenu7,MF_STRING,IDM_SELECTION_23,"S-Curve");

	
	MenuSelect = select + 40000;
	CheckMenuItem(hSubMenu1,MenuSelect, MF_CHECKED);
	CheckMenuItem(hSubMenu2,MenuSelect, MF_CHECKED);
	CheckMenuItem(hSubMenu3,MenuSelect, MF_CHECKED);
	CheckMenuItem(hSubMenu4,MenuSelect, MF_CHECKED);
	CheckMenuItem(hSubMenu5,MenuSelect, MF_CHECKED);
	CheckMenuItem(hSubMenu6,MenuSelect, MF_CHECKED);
	CheckMenuItem(hSubMenu7,MenuSelect, MF_CHECKED);
}
//---------------------------------------------------------------------------
void CCrossfaderCurves8::ReleaseMenu()
{
	DestroyMenu(hSubMenu1);
	DestroyMenu(hSubMenu2);
	DestroyMenu(hSubMenu3);
	DestroyMenu(hSubMenu4);
	DestroyMenu(hSubMenu5);
	DestroyMenu(hSubMenu6);
	DestroyMenu(hSubMenu7);
	DestroyMenu(hMenu);
}
//---------------------------------------------------------------------------
void CCrossfaderCurves8::OnResize(HWND hDlg,int WndWidth,int WndHeight)
{
	Width = WndWidth;
	Height = WndHeight;

	Invalidate(hDlg);
}
//---------------------------------------------------------------------------
void CCrossfaderCurves8::OnCommand(HWND hDlg,WORD id)
{
	switch(id)
	{
		case IDM_SELECTION_1:
		case IDM_SELECTION_2:
		case IDM_SELECTION_3:
		case IDM_SELECTION_4:
		case IDM_SELECTION_5:
		case IDM_SELECTION_6:
		case IDM_SELECTION_7:
		case IDM_SELECTION_8:
		case IDM_SELECTION_9:
		case IDM_SELECTION_10:
		case IDM_SELECTION_11:
		case IDM_SELECTION_12:
		case IDM_SELECTION_13:
		case IDM_SELECTION_14:
		case IDM_SELECTION_15:
		case IDM_SELECTION_16:
		case IDM_SELECTION_17:
		case IDM_SELECTION_18:
		case IDM_SELECTION_19:
	    case IDM_SELECTION_20:
		case IDM_SELECTION_21:
		case IDM_SELECTION_22:
		case IDM_SELECTION_23:
		case IDM_SELECTION_24:
			CheckMenuItem(hMenu,MenuSelect, MF_UNCHECKED); // Décoche un Item de menu
			MenuSelect=id;
			CheckMenuItem(hMenu,MenuSelect, MF_CHECKED); // Coche un Item de menu
			select = MenuSelect - 40000;
			if(select==1 || select==17 || select == 18 || select == 19 || select == 21) lock_custom=false;
			else lock_custom=true;
			SetParamCurves(select);
			button1_down = false;
			Invalidate(hDlg);
			break;
	}
}
//--------------------------------------------------------------------------
BOOL CCrossfaderCurves8::Invalidate(HWND hDlg)
{
	BOOL bRes;
	
	bRes = InvalidateRect(hDlg,NULL,FALSE); // avec FALSE car on ne redessine pas le fond du plugin
			                                // avec NULL pour ressiner toute l'interface cliente (on aurait pu mettre &r également)
	return bRes;
}
//---------------------------------------------------------------------------
void CCrossfaderCurves8::OnMouseDown(HWND hDlg,int x,int y,int button)
{
	if (y>=rButtons.top && y<=rButtons.bottom)  // Ligne des boutons
	{
		if (x>=r6.left && x<=r6.right)  // Bouton 1 : Changement de courbes (pré-programmées)
		{
			button1_down=true;
			Invalidate(hDlg);
			GetCursorPos(&pt);
			TrackPopupMenu(hMenu,TPM_LEFTALIGN | TPM_LEFTBUTTON,pt.x,pt.y,0,hDlg,NULL);
		}

		if (x>=r7.left && x<=r7.right)   // Bouton 2 : Inversion de la courbe
		{
			button2_down=true;
			inverted = inverted ? 0 : 1;
			Invalidate(hDlg);
		}

		if (x>=r8.left && x<=r8.right)   // Bouton 3 : About?
		{
			button3_down=true;
			Invalidate(hDlg);
			ShowAbout(hDlg, true);
		}

		if (x>=r9.left && x<=r9.right)   // Bouton 4 : Sélection de l'autre courbe
		{
			button4_down=true;
			select_level1=!select_level1;
			Invalidate(hDlg);
		}
	}
	else if (y>=rAxes.top && y<=rAxes.bottom) // Déplacement des points des courbes dans la zone de tracé
	{
		if (x>=rAxes.left && x<=rAxes.right)
		{
			if (lock_custom==false)
			{
				mousedown_curves = true;
			}
		}
	}
}
//---------------------------------------------------------------------------
void CCrossfaderCurves8::OnMouseUp(HWND hDlg,int x,int y,int button)
{
		mousedown_curves=false;
		button1_down=false;
		button2_down=false;
		button3_down=false;
		button4_down=false;
		point1_down=false;
		point2_down=false;
		point0_down=false;
		
		Invalidate(hDlg);
}
//---------------------------------------------------------------------------
void CCrossfaderCurves8::OnMouseMove(HWND hDlg,int x,int y)
{	
	double dy = 0.0f; 

	// Vérifie que le curseur est dans la fenêtre du plugin 
	if((x > 0) && (x < Width) && (y > 0) && (y < Height))
	{
	   if(mousedown_curves) //Gestion du mode custom de l'interface graphique 
		{
			if (select==1) // mode custom 1
			{
                   if (x>=rAxes.left && x<=rAxes.right && y>=rAxes.top && y<rAxes.bottom-10)  // Bande du haut
				   {
					   if(select_level1) 
					   {
						   point1_down=true;
					       C1P1=(int)((float)(x-rAxes.left)/(float)(rAxes.right-rAxes.left)*100.0f);
						   V1P1=(int)((float)(rAxes.bottom-y)/(float)(rAxes.bottom-rAxes.top)*100.0f);
						   if (x> x1P2)
							{
								C1P1=(int)((float)(x1P2-rAxes.left)*(float)100/(float)(rAxes.right-rAxes.left));
								C1P2=C1P1;
							}
					   }
					   else 
					   {
						   point2_down=true;
					       C2P2=(int)((float)(x-rAxes.left)*(float)100/(float)(rAxes.right-rAxes.left));
						   V2P2=(int)((float)(rAxes.bottom-y)*(float)100/(float)(rAxes.bottom-rAxes.top));
						   if (x < x2P1)
							{
								C2P2=(int)((float)(x2P1-rAxes.left)*(float)100/(float)(rAxes.right-rAxes.left));
								C2P1=C2P2;
							}
					   }
				   }
				   else if (x>=rAxes.left && x<=rAxes.right && y>=rAxes.bottom-10 && y<=rAxes.bottom+10) // Bande du bas
				   {
					   
					   if(select_level1) 
					   {
						   point2_down=true;
						   C1P2=(int)((float)(x-rAxes.left)/(float)(rAxes.right-rAxes.left)*100.0f);
						    if (x < x1P1)
							{
								C1P1=(int)(float(x1P1-rAxes.left)/float(rAxes.right-rAxes.left)*100.0f);
								C1P2=C1P1;
							}
						  
					   }
					   else 
					   {
						   point1_down=true;
					       C2P1=(int)(float(x-rAxes.left)/float(rAxes.right-rAxes.left)*100.0f);
						   if (x>x2P2)
							{
								C2P2=(int)(float(x2P2-rAxes.left)/float(rAxes.right-rAxes.left)*100.0f);
								C2P1=C2P2;
							}
					   }
					   Invalidate(hDlg);
				   }
			}
			else if (select== 17) // mode custom2
			{
                   if (x>=rAxes.left && x<=rAxes.right && y>=rAxes.top && y<=rAxes.bottom)  // Bande du haut
				   {
					   point0_down=true; 
					   dy = (double(rAxes.bottom)-double(y))/(double(rAxes.bottom)-double(rAxes.top));
					   VP0 = float ((1.0f - dy) * 100.0f);
					   Invalidate(hDlg);
				   }
			}
			else if (select== 18) // mode custom2
			{
                   if (x>=rAxes.left && x<=rAxes.right && y>=rAxes.top && y<=rAxes.bottom)  // Bande du haut
				   {
					   point0_down=true;
					   dy = (double(rAxes.bottom)-double(y))/(double(rAxes.bottom)-double(rAxes.top));
					   VP0 = float (dy * 100.0f);
					   Invalidate(hDlg);
				   }
			}
			else if (select == 19) // mode custom3
			{
                   if (x>=rAxes.left && x<=rAxes.right && y>=rAxes.top && y<=rAxes.bottom)  // Bande du haut
				   {
					   point0_down=true;
					   dy = (double(rAxes.bottom) - double(y))/(double(rAxes.bottom) - double(rAxes.top));
					   VP0 = float(pow(dy, 2) * 100.0f); // lent au debut puis rapide
					   Invalidate(hDlg);
				   }
			}
			else if (select == 21) // mode custom4
			{
                   if (x>=rAxes.left && x<=rAxes.right && y>=rAxes.top && y<=rAxes.bottom)  // Bande du haut
				   {
					   point0_down=true;  
					   dy = (double(rAxes.bottom) - double(y))/(double(rAxes.bottom) - double(rAxes.top));
					   VP0 = float(pow(1.0f - dy, 2) * 100.0f); // lent au debut puis rapide
					   Invalidate(hDlg);
				   }
			}
		}
	}
}
//--------------------------------------------------------------------------
void CCrossfaderCurves8::OnMouseLeave(HWND hDlg)
{
	mouseOver = false;
	Invalidate(hDlg);
}
//--------------------------------------------------------------------------
void CCrossfaderCurves8::OnPaint(HDC hDC)
{
	BOOL bRes = FALSE;
	HDC hMemDC = NULL;
	HBITMAP hMemBitmap = NULL;
	HBITMAP hOldBitmap = NULL;
	
	//Défini le cadre général du plugin
	RECT r;
	r.left=0;
	r.top=0;
	r.right=Width;
	r.bottom=Height;

	// Create double-buffer
	hMemDC = CreateCompatibleDC (hDC);
	hMemBitmap = CreateCompatibleBitmap (hDC, r.right-r.left, r.bottom-r.top);
	hOldBitmap = (HBITMAP) SelectObject (hMemDC, hMemBitmap);

	// Now we do all our drawing in memDC instead of in hDC
	DrawInterface(hMemDC, &r);

	// We copy the hMemDC to the hDC
	bRes = BitBlt(hDC,0,0,r.right,r.bottom,hMemDC,0,0,SRCCOPY);

	// Always select the old bitmap back into the device context
	SelectObject(hMemDC,hOldBitmap); // delete our doublebuffer
	DeleteObject(hMemBitmap);
	DeleteDC(hMemDC);
}
//--------------------------------------------------------------------------
void CCrossfaderCurves8::DrawInterface(HDC hDC, RECT *r)
{	
	HRESULT hr;

	int quarter_curves_XF;
	int quarter3_curves_XF;
	int largeur_bouton;
	int inter_espace;
	
	SetBkMode(hDC, TRANSPARENT);

	// Zone des boutons
	rButtons.top = r->top  + 10;
	rButtons.left = r->left;
	rButtons.right = r->right;
	rButtons.bottom = r->top + 40;

	if (select==1) // mode custom
	{
		NB_BUTTONS = 4;
	}
	else
	{
		NB_BUTTONS = 3;
	}

	largeur_bouton=(int) ((float)(rButtons.right - rButtons.left)/(float) ((float)NB_BUTTONS + 0.4));
	inter_espace=(int)((float)(rButtons.right - rButtons.left - NB_BUTTONS*largeur_bouton)/(float) (NB_BUTTONS+1));


	DrawButton(hDC,&r6, rButtons, button1_down,"XF_Curves",largeur_bouton,inter_espace);
	DrawButton(hDC,&r7, r6,(inverted==TRUE),"XF_Hamster",largeur_bouton,inter_espace);
	DrawButton(hDC,&r8, r7, button3_down,"About?",largeur_bouton,inter_espace);
	if (select==1) // mode custom
	{
		if (select_level1 == true) DrawButton(hDC,&r9, r8,button4_down,"Level 1",largeur_bouton,inter_espace);
		else DrawButton(hDC,&r9, r8,button4_down,"Level 2",largeur_bouton,inter_espace);
	}
	
	
	// Cadre blanc en fond noir		
	rCurves.top    = rButtons.bottom + 10;
	rCurves.bottom = r->bottom - 10;
	rCurves.left   = r->left + 5;
	rCurves.right  = r->right - 5 ;

	GridHeight = rCurves.bottom - rCurves.top; 
	GridWidth = rCurves.right - rCurves.left;

	if (GridWidth > GridHeight) 
	{
	    float deltaborder = float(GridWidth - GridHeight +10) / 2.0f;
		rCurves.left  = int(r->left + deltaborder);
		rCurves.right = int(r->right - deltaborder);
	} 

	GridWidth = rCurves.right - rCurves.left;

	// On gere les problemes d'affichage si c'est trop petit
	if(GridHeight <0) return;
	if(GridWidth <0) return;
	
	SelectObject(hDC, hBrushInterfaceCurvesBackground);
	SelectObject(hDC, hPenInterfaceCurvesBorder);
	Rectangle(hDC, rCurves.left, rCurves.top, rCurves.right, rCurves.bottom);
	
	// Définition des axes et du cadre interne en gris
	rAxes.top    = rCurves.top    + 30;
	rAxes.left   = rCurves.left   + 30;
	rAxes.right  = rCurves.right  - 30;
	rAxes.bottom = rCurves.bottom - 30;

	GridHeight = rAxes.bottom - rAxes.top; 
	GridWidth = rAxes.right - rAxes.left;

	// On gere les problemes d'affichage si c'est trop petit
	if(GridHeight <0) return;
	if(GridWidth <0) return;

	SelectObject(hDC, hPenAxes);

	MoveToEx(hDC, rAxes.left, rAxes.top, (LPPOINT) NULL);
	LineTo(hDC, rAxes.left, rAxes.bottom+5);
	MoveToEx(hDC, rAxes.left-5, rAxes.bottom, (LPPOINT) NULL);
	LineTo(hDC, rAxes.right+5, rAxes.bottom);
	MoveToEx(hDC, rAxes.right, rAxes.bottom+5, (LPPOINT) NULL);
	LineTo(hDC, rAxes.right, rAxes.top);
	
	// Petits tirets supplémentaires
	MoveToEx(hDC, rAxes.left-5, rAxes.top, (LPPOINT) NULL);
	LineTo(hDC, rAxes.left+5, rAxes.top);
	MoveToEx(hDC, rAxes.right-5, rAxes.top, (LPPOINT) NULL);
	LineTo(hDC, rAxes.right+5, rAxes.top);

    // Milieu du cadre central (milieu crossfader)
	xMiddle_curves_XF=(int)((rAxes.right-rAxes.left)/2);
	xMiddle_curves_XF+=rAxes.left;   
	MoveToEx(hDC, xMiddle_curves_XF, rAxes.bottom-5, (LPPOINT) NULL);
	LineTo(hDC, xMiddle_curves_XF, rAxes.bottom+5);

	// 1/4 du cadre central
	quarter_curves_XF=(int)((rAxes.right-rAxes.left)/4);
	quarter_curves_XF+=rAxes.left;   
	MoveToEx(hDC, quarter_curves_XF, rAxes.bottom-5, (LPPOINT) NULL);
	LineTo(hDC, quarter_curves_XF, rAxes.bottom+5);
    
	// 3/4 du cadre central
	quarter3_curves_XF=(int)((rAxes.right-rAxes.left)*3/4);
	quarter3_curves_XF+=rAxes.left;   
	MoveToEx(hDC, quarter3_curves_XF, rAxes.bottom-5, (LPPOINT) NULL);
	LineTo(hDC, quarter3_curves_XF, rAxes.bottom+5);

	// Milieu du level	
	yMiddle_curves_level=(int)((rAxes.bottom-rAxes.top)/2);
	yMiddle_curves_level+=rAxes.top;   
	MoveToEx(hDC, rAxes.left-5, yMiddle_curves_level, (LPPOINT) NULL);
	LineTo(hDC, rAxes.left+5,yMiddle_curves_level);
	MoveToEx(hDC, rAxes.right-5, yMiddle_curves_level, (LPPOINT) NULL);
	LineTo(hDC, rAxes.right+5,yMiddle_curves_level);

	// Legende en gris	
	SetTextColor(hDC, RGB(150,150,150)); 
	TextOut(hDC,rAxes.left-2,rAxes.bottom+7,"0",(int)strlen("0"));
	TextOut(hDC,xMiddle_curves_XF-14,rAxes.bottom+7,"0.5",(int)strlen("0.5"));
	TextOut(hDC,rAxes.right-8,rAxes.bottom+7,"1",(int)strlen("1"));

	// Legende des courbes
	if(inverted==TRUE)
	{
		SetTextColor(hDC, RGB(172,47,55));
		TextOut(hDC,rAxes.left -20,rAxes.top-20,"Level 2",(int)strlen("Level 2"));
		SetTextColor(hDC, RGB(21,108,137));
		TextOut(hDC,rAxes.right-30,rAxes.top-20,"Level 1",(int)strlen("Level 1"));
	}
	else
	{
		SetTextColor(hDC, RGB(21,108,137));
		TextOut(hDC,rAxes.left -20,rAxes.top-20,"Level 1",(int)strlen("Level 1"));
		SetTextColor(hDC, RGB(172,47,55));
		TextOut(hDC,rAxes.right-30,rAxes.top-20,"Level 2",(int)strlen("Level 2"));
	}

	SetTextColor(hDC, RGB(255, 251, 170));
	TextOut(hDC,xMiddle_curves_XF-10,rAxes.top-20,"Sum",(int)strlen("Sum")); 

	if (mousedown_curves && select != 1)
	{
		
		SetTextColor(hDC, RGB(255, 255, 255));
		sprintf(strVPO, "%.2f", VP0);
		TextOut(hDC, xMiddle_curves_XF - 70, rAxes.top - 20, strVPO, (int)strlen(strVPO));
		sprintf(strVPO, "%.2f", level[0][50]);
		TextOut(hDC, xMiddle_curves_XF + 60, rAxes.top - 20, strVPO, (int)strlen(strVPO));
	}
 
	//*********************************************************
	// Tracé des courbes corresponantes
	//*********************************************************
	
	SetParamCurvesGraphic(hDC,select);
	
	hr = DrawCurves(hDC);
}
//--------------------------------------------------------------------------
void CCrossfaderCurves8::DrawButton(HDC hDC,RECT *rCurrent,RECT rPrevious, bool ButtonDown, char *text,int largeur_bouton,int inter_espace)
{
	rCurrent->top    = rPrevious.top;
	rCurrent->bottom = rPrevious.bottom;
	if(rPrevious.left == 0)
	{
		rCurrent->left   = inter_espace;
	}
	else 
	{
		rCurrent->left   = rPrevious.right + inter_espace;
	}
	rCurrent->right  = rCurrent->left + largeur_bouton;

	if(ButtonDown) // BorderColor and BackgroundColor when the button is down
	{
		SelectObject(hDC, hBrushButton2Background);
		SelectObject(hDC, hPenButton2Border);
	}
	else
	{
		SelectObject(hDC, hBrushButton1Background);
		SelectObject(hDC, hPenButton1Border);
	}

	Rectangle(hDC, rCurrent->left, rCurrent->top, rCurrent->right, rCurrent->bottom);

	COLORREF textcolor = ButtonDown ? RGB(0, 0, 0) : RGB(198, 198, 198);
	SetTextColor(hDC, textcolor);
	DrawText(hDC, text, -1, rCurrent, DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX);
}
//---------------------------------------------------------------------------
void CCrossfaderCurves8::ShowAbout(HWND hDlg, bool isToolTip)
{
	char msg[2048] = "";
	sprintf(msg, "Plugin name: %s\r\nAuthor: %s\r\nVersion: %s\r\nDescription: %s\r\n\nPlease note that non-linear curves are an approximation of real curves. In this plugin, it's a sampling of 101 dots with a precision of 2 decimals on the value.", _TAbout.PluginName, _TAbout.Author, _TAbout.Version, _TAbout.Description);

	if (isToolTip)
	{
		HWND ToolTipWnd = CreateWindow(TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX | TTS_BALLOON, 0, 0, 0, 0, hDlg, NULL, NULL, 0);

		if (ToolTipWnd != NULL)
		{
			SendMessage(ToolTipWnd, TTM_SETMAXTIPWIDTH, 0, 480);
			SendMessage(ToolTipWnd, TTM_ACTIVATE, TRUE, 0);

			TOOLINFO toolinfo;
			memset(&toolinfo, 0, sizeof(TOOLINFO));
			toolinfo.cbSize = sizeof(TOOLINFO);
			toolinfo.uFlags = TTF_SUBCLASS;
			toolinfo.rect = r8;
			toolinfo.hwnd = hWndPlugin;
			toolinfo.hinst = NULL;
			toolinfo.lpszText = msg;

			SendMessage(ToolTipWnd, TTM_ADDTOOL, 0, (LPARAM)&toolinfo);
		}
	}
	else
	{
		MessageBox(hDlg, msg, "VirtualDJ plugin", MB_OK);
	}
}
//--------------------------------------------------------------------------
void CCrossfaderCurves8::SetParamCurvesGraphic(HDC hDC,int select)
{
	switch(select)
	{
		case 1: // Custom
	       x1P1 = int(rAxes.left + float(C1P1)/100.0f *(rAxes.right-rAxes.left));
		   y1P1 = int(rAxes.bottom - float(V1P1)/100.0f *(rAxes.bottom-rAxes.top));
		   x1P2 = int(rAxes.left + float(C1P2)/100.0f *(rAxes.right-rAxes.left));
		   y1P2 = int(rAxes.bottom - float(V1P2)/100.0f *(rAxes.bottom-rAxes.top));
		   x2P1 = int(rAxes.left + float(C2P1)/100.0f *(rAxes.right-rAxes.left));
		   y2P1 = int(rAxes.bottom - float(V2P1)/100.0f *(rAxes.bottom-rAxes.top));
		   x2P2 = int(rAxes.left + float(C2P2)/100.0f *(rAxes.right-rAxes.left));
		   y2P2 = int(rAxes.bottom - float(V2P2)/100.0f *(rAxes.bottom-rAxes.top));
			break;
	
		case 17:
			xP0 = xMiddle_curves_XF;
			yP0 = int(rAxes.bottom - (1.0f - VP0/100.0f) * float(rAxes.bottom-rAxes.top));
			break;

		case 18:
			xP0 = xMiddle_curves_XF;
			yP0 = int(rAxes.bottom - VP0/100.0f * float(rAxes.bottom-rAxes.top));
			break;

		case 19:
			xP0 = xMiddle_curves_XF;
			yP0 = int(rAxes.bottom - (float)sqrt(VP0/100.0f) * float(rAxes.bottom-rAxes.top));
			break;

		case 21:
			xP0 = xMiddle_curves_XF;
			yP0 = int(rAxes.bottom - (1.0f - (float)sqrt(VP0/100.0f)) * float(rAxes.bottom-rAxes.top));
			break;	
	}
}	
//--------------------------------------------------------------------------
void CCrossfaderCurves8::SetParamCurves(int select)
{
	switch(select)
	{
		case 1: // Linear Custom
			wsprintf(selectText,"Customize");
			type1=1;
			type2=1;
			break;
	
		case 2: // Full
			wsprintf(selectText,"Full");
			type1=1;
			type2=1;
			C1P1=50;
			V1P1=100;
			C1P2=100;
			V1P2=0;
			C2P1=0;
			V2P1=0;
			C2P2=50;
			V2P2=100;
			break;

	case 3: // Smooth
			wsprintf(selectText,"Smooth");
			type1=1;
			type2=1;
			C1P1=30;
			V1P1=100;
			C1P2=100;
			V1P2=0;
			C2P1=0;
			V2P1=0;
			C2P2=70;
			V2P2=100;
		    break;

	case 4: // Cut
			wsprintf(selectText,"Cut");
			type1=1;
			type2=1;
			C1P1=50;
			V1P1=100;
			C1P2=50;
			V1P2=0;
			C2P1=50;
			V2P1=0;
			C2P2=50;
			V2P2=100;
		   break;

	case 5: // Scratch
			wsprintf(selectText,"Scratch");
			type1=1;
			type2=1;
			C1P1=100;
			V1P1=100;
			C1P2=100;
			V1P2=0;
			C2P1=0;
			V2P1=0;
			C2P2=0;
			V2P2=100;
			break;

	case 6: // Cross (X)
			wsprintf(selectText,"Cross");
			type1=1;
			type2=1;
			C1P1=0;
			V1P1=100;
			C1P2=100;
			V1P2=0;
			C2P1=0;
			V2P1=0;
			C2P2=100;
			V2P2=100;
			break;

	case 7: // New 2 (1)
			wsprintf(selectText,"New 2 (1)");
			type1=1;
			type2=1;
			C1P1=50;
			V1P1=100;
			C1P2=50;
			V1P2=0;
			C2P1=0;
			V2P1=0;
			C2P2=50;
			V2P2=100;
		   break;

	case 8: // New 2 (2)
			wsprintf(selectText,"New 2 (2)");
			type1=1;
			type2=1;
			C1P1=50;
			V1P1=100;
			C1P2=100;
			V1P2=0;
			C2P1=50;
			V2P1=0;
			C2P2=50;
			V2P2=100;
		   break;

	case 9: // V
			wsprintf(selectText,"V");
			type1=1;
			type2=1;
			C1P1=0;
			V1P1=100;
			C1P2=50;
			V1P2=0;
			C2P1=50;
			V2P1=0;
			C2P2=100;
			V2P2=100;
		   break;

	case 10: // Smooth V
			wsprintf(selectText,"Smooth V");
			type1=1;
			type2=1;
			C1P1=0;
			V1P1=100;
			C1P2=67;
			V1P2=0;
			C2P1=33;
			V2P1=0;
			C2P2=100;
			V2P2=100;
		   break;

	case 11: // Long Full
			wsprintf(selectText,"Long Full");
			type1=1;
			type2=1;
			C1P1=75;
			V1P1=100;
			C1P2=100;
			V1P2=0;
			C2P1=0;
			V2P1=0;
			C2P2=25;
			V2P2=100;
		   break;

		case 12:
			wsprintf(selectText,"High Circle Max");
			type1=2;
			type2=2;
			VP0=0.0f;
			break;

		case 13:
			wsprintf(selectText,"High Circle 3/4");
			type1=2;
			type2=2;
			VP0=38.0f;
			break;

		case 14:
			wsprintf(selectText,"High Circle -3dB");
			type1=2;
			type2=2;
			VP0=60.0f;
			break;

		case 15:
			wsprintf(selectText,"Low Circle");
			type1=3;
			type2=3;
			VP0=87.0f;
			break;

	    case 16:
			wsprintf(selectText,"Log");
			type1=4;
			type2=4;
			break;

		case 17:
			wsprintf(selectText,"Customize");
			type1=2;
			type2=2;
			break;

		case 18:
			wsprintf(selectText,"Customize");
			type1=3;
			type2=3;
			break;

		case 19:
			wsprintf(selectText,"Customize");
			type1=5;
			type2=5;
			break;

		case 20:
			wsprintf(selectText,"High Power Medium");
			type1=5;
			type2=5;
			VP0=10.0f;
			break;

		case 21:
			wsprintf(selectText,"Customize");
			type1=6;
			type2=6;
			break;

		case 22:
			wsprintf(selectText,"Low Power Medium");
			type1=6;
			type2=6;
			VP0=10.0f;
			break;

		case 23:
			wsprintf(selectText,"S-Curve");
			type1=7;
			type2=7;
			break;

		case 24:
			wsprintf(selectText, "High Power Very Low");
			type1 = 5;
			type2 = 5;
			VP0 = 0.80f;
			break;
	}
}
//--------------------------------------------------------------------------
HRESULT CCrossfaderCurves8::DrawCurves(HDC hDC)
{
	HRESULT hr = S_OK;
	
	RemplirTableauLevel1(type1);
	RemplirTableauLevel2(type2);
		
	DrawLevel(hDC);
	DrawPointsLevel(hDC);
	

	if (mousedown_curves==true || button1_down==true || button2_down==true)
	{
		hr = SetCrossfaderCurve();	
	}
	
	return hr;
}
//--------------------------------------------------------------------------
void CCrossfaderCurves8::DrawLevel(HDC hDC)
{
	// Fonction qui trace la coube level point par point (=> dessine toutes les courbes)
	
	int xP,yP;
	int xP_tmp,yP_tmp;
	int XF;
	int lv;
	float SumLevel;

	
	for(lv=0;lv<2;lv++)
	{
		for(XF=0;XF<=100;XF++)
		{
			xP = rAxes.left + (int)(float(XF) / 100.0f *(float)(rAxes.right-rAxes.left));
			yP = rAxes.bottom - (int)(level[lv][XF]*(float)(rAxes.bottom-rAxes.top));
			
			if(XF==0)
			{
				xP_tmp = xP;
				yP_tmp = yP;
			}
			
			if (lv==1) SelectObject(hDC, hPenLevel1);
			else SelectObject(hDC, hPenLevel2);

			MoveToEx(hDC, xP, yP, (LPPOINT) NULL);
			LineTo(hDC, xP_tmp, yP_tmp);
			
			xP_tmp = xP;
			yP_tmp = yP;
		}

	}

	// Sum of both curves
	for(XF=0;XF<=100;XF++)
	{
		SumLevel = (level[0][XF] + level[1][XF])/2.0f;

		xP = rAxes.left + (int)(float(XF) / 100.0f *(float)(rAxes.right-rAxes.left));
		yP = rAxes.bottom - (int)(SumLevel*(float)(rAxes.bottom-rAxes.top));

		if(XF==0)
		{
			xP_tmp = xP;
			yP_tmp = yP;
		}
			
		SelectObject(hDC, hPenLevel3);
			
		MoveToEx(hDC, xP, yP, (LPPOINT) NULL);
		LineTo(hDC, xP_tmp, yP_tmp);
			
		xP_tmp = xP;
		yP_tmp = yP;
		
	}
}
//--------------------------------------------------------------------------
void CCrossfaderCurves8::DrawPointsLevel(HDC hDC) 
{		
	if(lock_custom==false)
	{
		if(select == 1)
		{
			if (select_level1==true)
			{
				// Point 1
				if(point1_down==false) SelectObject(hDC, hPointBorderUp);
				else SelectObject(hDC, hPointBorderDown);
				Rectangle(hDC, x1P1-3, y1P1-3, x1P1+3, y1P1+3);

				// Point 2
				if(point2_down==false) SelectObject(hDC, hPointBorderUp);
				else SelectObject(hDC, hPointBorderDown);
				Rectangle(hDC, x1P2-3, y1P2-3, x1P2+3, y1P2+3);
			}
			else
			{
				// Point 1
				if(point1_down==false) SelectObject(hDC, hPointBorderUp);
				else SelectObject(hDC, hPointBorderDown);
				Rectangle(hDC, x2P1-3, y2P1-3, x2P1+3, y2P1+3);

				// Point 2
				if(point2_down==false) SelectObject(hDC, hPointBorderUp);
				else SelectObject(hDC, hPointBorderDown);
				Rectangle(hDC, x2P2-3, y2P2-3, x2P2+3, y2P2+3);
			}
		}
		else if(select == 17 || select == 18 || select == 19 || select == 21)
		{
			// Point Reference
			if(point0_down==false) SelectObject(hDC, hPointBorderUp);
			else SelectObject(hDC, hPointBorderDown);
			Rectangle(hDC, xP0-3, yP0-3, xP0+3, yP0+3);
		}
	}
}
#endif

