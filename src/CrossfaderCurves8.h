#ifndef CROSSFADERCURVES8_H
#define CROSSFADERCURVES8_H

#define _CRT_SECURE_NO_WARNINGS

#include "VdjPlugin8.h"

#include "resource.h"
#include <commctrl.h>

#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>					// Pour la racine carrée


#define CROSSFADERCURVES8_GUI

//---------------------------------------------------------------------------
// Class definition
//---------------------------------------------------------------------------
class CCrossfaderCurves8 : public IVdjPlugin8
{
public:
	HRESULT VDJ_API OnLoad();
	HRESULT VDJ_API OnGetPluginInfo(TVdjPluginInfo8 *infos);
	ULONG   VDJ_API Release();
	HRESULT VDJ_API OnParameter(int id);
	HRESULT VDJ_API OnGetParameterString(int id, char *outParam, int outParamSize);
	HRESULT VDJ_API OnGetUserInterface(TVdjPluginInterface8 *pluginInterface);
	
private:
	bool RemplirTableauLevel1(int type);
	bool RemplirTableauLevel2(int type);
	HRESULT SetCrossfaderCurve();
	float square(float x);

	int select;
	int inverted; // inversion du crossfader
	char selectText[128];
	float level1[101];
	float level2[101];
	float level[2][101];
	int type1,type2; // Type de courbe {lineaire, parabolique) pour l'instant que lineaire
	int C1P1,C1P2,C2P1,C2P2;   // Crossfader € [0,100]: C1 for level 1 && C2 for level 2
							   // avec C1P1<=C1P2 && C2P1<=C2P2                           
	int V1P1,V1P2,V2P1,V2P2;   // Volume € [0,100]: V1 for level 1 && V2 for level 2
							   // avec V1P1>=V1P2 && V2P1<=V2P2
	float VP0; // Volume transformation factor


	#if (defined(CROSSFADERCURVES8_GUI))
		VDJ_WINDOW hWndPlugin;
		VDJ_WINDOW hWndParent;
		#if (defined(VDJ_WIN))
		    void CreateWindowGUI(HINSTANCE hInstance,HWND hWndParent,int Width, int Height,HWND *hWndPlugin);
			void DestroyWindowGUI();
			void CloseWindowGUI(HWND hDlg);
			static INT_PTR CALLBACK DialogProcStatic(HWND, UINT, WPARAM, LPARAM);
			virtual INT_PTR CALLBACK DialogProc(HWND , UINT, WPARAM, LPARAM);
			void InitInterface(HWND);
			void ReleaseInterface(HWND);
			void InitMenu();
			void ReleaseMenu();
			void OnMouseDown(HWND hDlg, int x,int y,int button);
			void OnMouseUp(HWND hDlg, int x,int y,int button);
			void OnMouseMove(HWND hDlg,int x,int y);
			void OnResize(HWND hDlg,int WndWidth,int WndHeight);
			void OnCommand(HWND hDlg, WORD id);
			void OnPaint(HDC hDC);
			void DrawInterface(HDC hDC, RECT *r);
			BOOL Invalidate(HWND hDlg);
			HRESULT DrawCurves(HDC hDC);
			void SetParamCurves(int select);
			void SetParamCurvesGraphic(HDC hDC,int select);
			void DrawButton(HDC hDC,RECT *rCurrent,RECT rPrevious,bool status, char *text,int largeur_bouton,int inter_espace);
			void DrawPointsLevel(HDC hDC);
			void DrawLevel(HDC hDC);
			void ShowAbout(HWND hDlg);

			struct TVdjPluginInfo8  _TAbout;
			
			HMENU hMenu;
			HMENU hSubMenu1;
			HMENU hSubMenu2;
			HMENU hSubMenu3;
			HMENU hSubMenu4;
			HMENU hSubMenu5;
			HMENU hSubMenu6;
			HMENU hSubMenu7;
			int MenuSelect;

			POINT pt;
			HBRUSH	hBrushInterfaceCurvesBackground;
			HPEN	hPenInterfaceCurvesBorder,hPenAxes;
			HBRUSH  hBrushButton1Background,hBrushButton2Background;
			HPEN    hPenButton1Border,hPenButton2Border;
			HPEN    hPenLevel1,hPenLevel2,hPenLevel3;
			HPEN    hPointBorderUp,hPointBorderDown;
			
			RECT rButtons;
			RECT rCurves; // Fond noir des courbes
			RECT rAxes; // Définition des axes 
			RECT r6; // Bouton 1
			RECT r7; // Bouton 2
			RECT r8; // Bouton 4
			RECT r9; // Bouton 3

			int NB_BUTTONS;
			int Width;
			int Height;
			int GridHeight;
			int GridWidth;
			bool lock_custom;
			bool select_level1; // Sélection de la courbe du volume (level 1 ou 2)
			bool mousedown_curves; // Appui sur le point des courbes avec la souris
			bool button1_down;
			bool button2_down;
			bool button4_down;
			bool point1_down;
			bool point2_down;
			bool point0_down;
			int xMiddle_curves_XF;
			int yMiddle_curves_level;
			int x1P1,y1P1,x1P2,y1P2;
			int x2P1,y2P1,x2P2,y2P2;
			int xP0,yP0;
			char strVPO[12];
		#endif
	#endif

	typedef enum _ID_Interface
	{
		ID_INIT,
		ID_SWITCH_1,
		ID_CUSTOM_1,
		ID_CUSTOM_2,
		ID_CUSTOM_3,
		ID_CUSTOM_4,
		ID_CUSTOM_5,
		ID_CUSTOM_6,
		ID_CUSTOM_7,
		ID_CUSTOM_8,
		ID_CUSTOM_9,
		ID_CUSTOM_10
	} ID_Interface;

	#define CARRE(x) x*x
};

#endif /* CROSSFADERCURVES8_H */
