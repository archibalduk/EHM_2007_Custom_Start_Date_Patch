// CSD_Patch.cpp : main project file.

#include "stdafx.h"
#include "CSD_Form.h"

using namespace CSDPatch;

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
	// Enabling Windows XP visual effects before any controls are created
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 

	// Create the main window and run it
	Application::Run(gcnew CSD_Form());
	return 0;
}
