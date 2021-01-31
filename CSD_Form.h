#pragma once
#define _WIN32_WINNT _WIN32_WINNT_WINXP // Enables Windows XP compatibility
#include <windows.h>
#include <Tlhelp32.h>
#include <cstdlib>
#include <iostream>
#include <Shellapi.h>
#include <time.h>

// Enables the ehm2007.exe process to be manipulated
DWORD GetPIDForProcess (wchar_t* process)
{
    BOOL            working=0;
    PROCESSENTRY32 lppe={0};
    DWORD            targetPid=0;
    HANDLE hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS ,0);
    if (hSnapshot) 
	{
		lppe.dwSize=sizeof(lppe);
		working=Process32First(hSnapshot,&lppe);
		while (working)
		{
			if(wcscmp(lppe.szExeFile,process)==0)
			{
				targetPid=lppe.th32ProcessID;
				break;
			}
			working=Process32Next(hSnapshot,&lppe);
		}
	}
    CloseHandle( hSnapshot );
    return targetPid;
}

// Protect/Unprotect the memory
bool SetAccessLevel(HANDLE &ehm2007_exe, bool protect, int offset, int size)
{	
	DWORD flNewProtect = 0;		// Buffer to store the state of the new access protection
	DWORD lpflOldProtect = 0;	// Buffer to store the state of the previous access protection
	//int offset = 0xA1A000;		// Offset of target memory to protect
	//int size = 0x1D3000;		// Size of memory to protect (0x1D3000 = 1912832 bytes)

	// Set the new access protection according to whether the protect flag as been set
	if(protect == true)
		flNewProtect = PAGE_READONLY;
	else
		flNewProtect = PAGE_READWRITE;	

	if(!VirtualProtectEx(ehm2007_exe, (void *)offset, size, flNewProtect, &lpflOldProtect))
		return false;
	else
		return true;
}

namespace CSDPatch {

	char* ehmVersion;
	char* ehmEdition;
	int patchtype = 3;
	int offsetWorldChampionships;
	int offsetOlympics;
	int offsetScheduleMinimumYear;
	int ErrorCount = 0;
	int OffsetErrorCount = 0;
	int WarningCount = 0;
	int i = 0;
	int eLicenseBuffer;
	int cdBuffer;
	int crackBuffer;

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for CSD_Form
	/// </summary>
	public ref class CSD_Form : public System::Windows::Forms::Form
	{
	public:
		CSD_Form(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~CSD_Form()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Button^  btnApply;
	private: System::Windows::Forms::TextBox^  textOutcome;

	private: System::Windows::Forms::NumericUpDown^  numericStartYear;
	private: System::Windows::Forms::Button^  btnWindowed;
	private: System::Windows::Forms::Button^  btnStandard;
	private: System::Windows::Forms::TextBox^  textStatus;
	private: System::Windows::Forms::TextBox^  textEHMDetails;


	private: System::Windows::Forms::CheckBox^  checkAutoExit;
	private: System::Windows::Forms::Label^  labelStatus;
	private: System::Windows::Forms::Label^  labelOutcome;
	private: System::Windows::Forms::Label^  labelStartYear;
	private: System::Windows::Forms::LinkLabel^  linkLabelTBL;




	private: System::Windows::Forms::Label^  labelTitle;
	private: System::Windows::Forms::Label^  labelSubTitle;
	private: System::Windows::Forms::StatusStrip^  statusStrip1;
	private: System::Windows::Forms::ToolStripStatusLabel^  toolStripStatusAuthor;
	private: System::Windows::Forms::ToolStripStatusLabel^  toolStripStatusVersion;
	private: System::Windows::Forms::TextBox^  textHelp1;
	private: System::Windows::Forms::TextBox^  textHelp2;
	private: System::Windows::Forms::TextBox^  textHelp3;
	private: System::Windows::Forms::TextBox^  textHelp4;
	protected: System::Windows::Forms::PictureBox^  pictureLogo;
	private: 

	private: 



	protected: 

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

// Gets the current year in order to determine the default StartYear
void getStartYear(void)
	{
		time_t t = time(0);
		struct tm * now = localtime( & t );
		int StartYear = (now->tm_year + 1900);
		if (now->tm_mon < 8) { StartYear = StartYear - 1; }
		numericStartYear->Value = StartYear;
	}

// Reset the patch
void resetpatch(void)
{
	WarningCount = 0;
	ErrorCount = 0;
	OffsetErrorCount = 0;
	textOutcome->Clear();
	if (i > 0) { textStatus->AppendText(L"\r\n---------------------------------------------------------------------------------------------\r\n"); }
	ehmVersion = "ersion";
	ehmEdition = "Unrecognised";
	eLicenseBuffer = NULL;
	cdBuffer = NULL;
	crackBuffer = NULL;
}

// Load EHM either in small_screen or standard mode
bool loadEHM(bool Small_Screen)
{
	if (Small_Screen == true)
	{
		textStatus->AppendText(L"Loading EHM (-small_screen -windowed)...\r\n");
		textEHMDetails->Text = L"Loading EHM (-small_screen -windowed)...";
		ShellExecute(NULL, L"open", L"EHM2007.exe", L"-small_screen -windowed", NULL, SW_SHOW);
		textStatus->AppendText(L"Please wait whilst EHM loads...\r\n");
	}
	else
	{
		textStatus->AppendText(L"Loading EHM...\r\n");
		textEHMDetails->Text = L"Loading EHM...";
		ShellExecute(NULL, L"open", L"EHM2007.exe", NULL, NULL, SW_SHOW);
		textStatus->AppendText(L"Please wait whilst EHM loads...\r\n");
	}

	Sleep(6000);	// Wait six seconds in order to allow EHM to load before applying the memory patch

	return(0);
}

// Check that EHM is running and detect which version
void detectEHM(void)
{
	void EnableDebugPriv();
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, false, GetPIDForProcess(L"ehm2007.exe"));

	if(GetPIDForProcess(L"ehm2007.exe") == 0) {
		ErrorCount++;
		textEHMDetails->Text = L"EHM is not running.";
		textStatus->AppendText(L"ERROR: EHM is not running. Patch aborted.\r\n");
	}

	else
	{
		ReadProcessMemory(hProc, (void *)0x1301670, &eLicenseBuffer, 5, NULL);
		ReadProcessMemory(hProc, (void *)0x12FBFE0, &cdBuffer, 5, NULL);
		ReadProcessMemory(hProc, (void *)0x24991EA, &crackBuffer, 8, NULL);

		if (eLicenseBuffer == 774909440 || eLicenseBuffer == 774909491)
		{
			ehmVersion = "3.0.4";
			patchtype = 1;
			if	(crackBuffer == 1667845221) { ehmEdition = "eLicense"; }
			else
			{
				ehmEdition = "Cracked";
				WarningCount++;
				textStatus->AppendText(L"WARNING: The patch has not been tested with the cracked version.\r\nUse the cracked version at your own risk. TBL does not support piracy.\r\n");
			}
		}
		else if (cdBuffer == 774909491)
		{
			ehmVersion = "3.0.4";
			patchtype = 2;
			ehmEdition = "CD-ROM";
			WarningCount++;
			textStatus->AppendText(L"WARNING: The patch is not currently compatible with the CD-ROM version.\r\n");
		}
		else
		{
			WarningCount++;
			textStatus->AppendText(L"WARNING: Unrecognised version of EHM.\r\nThe patch might not work.\r\nDebug info:\r\n1) eLicense: " + eLicenseBuffer + L"\r\n2) Crack: " + crackBuffer + L"\r\n3) CD-ROM: " + cdBuffer + L"\r\n");
		}
		textEHMDetails->Text = gcnew String(ehmEdition) + " v" + gcnew String(ehmVersion);
	}
}

// The main memory patch which writes the data to the ehm2007.exe process
WORD memorypatch(WORD StartYear)
{
	void EnableDebugPriv();
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, false, GetPIDForProcess(L"ehm2007.exe"));

	if (ErrorCount > 0) { return(0); }	// Don't apply patch if errors were encountered loading/detecting EHM

	WORD NextYear = StartYear + 1;

	const int offsetELicenseSize = 15;
	const int offsetELicense[offsetELicenseSize] = {
		0x7D11C0,	// Start date
		0x5496A6,	// CHL draft
		0x5F08A2,	// Player career history
		0x89A953,	// Schedule arrow buttons (pre-1980 start dates)
		0x89AD5A,	// Schedule arrow buttons (pre-1980 start dates)
		0x916ED7,	// 12 February crash
		0x9196D7,	// 1 October crash
		0x920612,	// 28 February crash
		0x920DD5,	// Finland .cpp error
		0x9213BF,	// Finland crash
		0x921734,	// 15 April crash
		0x925F83,	// 29 June crash
		0x944324,	// 25 May crash
		0x9450C7,	// Russia .cpp error 1
		0x945364	// Russia .cpp error 2
	};
		
	const int offsetCDROMSize = 3;
	const int offsetCDROM[offsetCDROMSize] = {
		0xC6456E,	// Start date (new game screen)
		0xC651C0,	// Start date monitoring 1
		0xC651CC	// Start date monitoring 2
	};		

	textStatus->AppendText(L"Applying "  + (int) numericStartYear->Value + L" start year...\r\n");
	
	switch (patchtype)
	{
	case 1:
		textStatus->AppendText(L"Patch type 1 (eLicense)\r\n");
		offsetWorldChampionships = 0x91E517;		// World Championships
		offsetOlympics = 0x913AB8;					// Olympics
		offsetScheduleMinimumYear = 0x89A1E5;		// Schedule minimum year value (default is 1980)
		for(i = 0; i < offsetELicenseSize; i++)
		{
			if (!WriteProcessMemory(hProc, (void *)offsetELicense[i], &StartYear, sizeof(StartYear), NULL)) { OffsetErrorCount++; }
		}
		break;
	case 2:
		textStatus->AppendText(L"Patch type 2 (CD-ROM)\r\n");
		offsetWorldChampionships = 0x91E517;		// World Championships
		offsetOlympics = 0x913AB8;					// Olympics
		offsetScheduleMinimumYear = 0x89A1E5;		// Schedule minimum year value (default is 1980)
		for(i = 0; i < offsetCDROMSize; i++)
		{
			if (!WriteProcessMemory(hProc, (void *)offsetCDROM[i], &StartYear, sizeof(StartYear), NULL)) { OffsetErrorCount++; }
		}
		break;
	default:
		textStatus->AppendText(L"Patch type 3 (default)\r\n");
		offsetWorldChampionships = 0x91E517;		// World Championships
		offsetOlympics = 0x913AB8;					// Olympics
		offsetScheduleMinimumYear = 0x89A1E5;		// Schedule minimum year value (default is 1980)
		for(i = 0; i < offsetELicenseSize; i++)
		{
			if (!WriteProcessMemory(hProc, (void *)offsetELicense[i], &StartYear, sizeof(StartYear), NULL)) { OffsetErrorCount++; }
		}
	}

	// Write Next Year values
	if (!WriteProcessMemory(hProc, (void *)offsetWorldChampionships, &NextYear, sizeof(NextYear), NULL)) { OffsetErrorCount++; }	// World Championships
	i++;	// Add one to the i counter to account for the Next Year value.
	if (!WriteProcessMemory(hProc, (void *)offsetOlympics, &NextYear, sizeof(NextYear), NULL)) { OffsetErrorCount++; }				// Olympics
	i++;	// Add one to the i counter to account for the Next Year value.

	// Write schedule minimum year value if the start year is earlier than 1980
	if(StartYear < 1980) {
		WORD MininumYear = 1901;
		//SetAccessLevel(hProc, false, offsetScheduleMinimumYear, sizeof(scheduleMininumYear));
		if (!WriteProcessMemory(hProc, (void *)offsetScheduleMinimumYear, &MininumYear, sizeof(MininumYear), NULL)) { OffsetErrorCount++; }	// Schedule minimum year value (default is 1980)
		//SetAccessLevel(hProc, true, offsetScheduleMinimumYear, sizeof(scheduleMininumYear));
		i++;	// Add one to the i counter to account for this value.
	}

	if (OffsetErrorCount == 0 && patchtype == 2)
	{
		ErrorCount++;
		textStatus->AppendText(L"ERROR: The patch is not currently compatible with the CD-ROM version.\r\n");
	}
	else if (OffsetErrorCount > 0)
	{
		ErrorCount++;
		textStatus->AppendText(L"ERROR: " + OffsetErrorCount + " of " + i + L" values failed.\r\n");
	}

	return(0);
}

void patchfinish(void)
{
	switch(ErrorCount)
	{
	case 0:
		textStatus->AppendText(L"Patch successful.");
		textOutcome->Text = L"Patch successful.";
		if (WarningCount > 0) { textStatus->AppendText(L"\r\nPlease note the " + WarningCount + " warning(s) listed above."); }
		if (checkAutoExit->Checked)
		{
			textStatus->AppendText(L"\r\nThe patch window will now automatically close...");
			Sleep(1250);	// Wait 1.25 seconds before closing down
			exit(0);
		}
		break;
	default:
		textStatus->AppendText(L"Patch failed.\r\n" + ErrorCount + L" error(s) and " + WarningCount + L" warning(s) encountered.");
		textOutcome->Text = L"Patch failed.";
	}
}


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(CSD_Form::typeid));
			this->btnApply = (gcnew System::Windows::Forms::Button());
			this->textOutcome = (gcnew System::Windows::Forms::TextBox());
			this->numericStartYear = (gcnew System::Windows::Forms::NumericUpDown());
			this->btnWindowed = (gcnew System::Windows::Forms::Button());
			this->btnStandard = (gcnew System::Windows::Forms::Button());
			this->textStatus = (gcnew System::Windows::Forms::TextBox());
			this->textEHMDetails = (gcnew System::Windows::Forms::TextBox());
			this->checkAutoExit = (gcnew System::Windows::Forms::CheckBox());
			this->labelStatus = (gcnew System::Windows::Forms::Label());
			this->labelOutcome = (gcnew System::Windows::Forms::Label());
			this->labelStartYear = (gcnew System::Windows::Forms::Label());
			this->linkLabelTBL = (gcnew System::Windows::Forms::LinkLabel());
			this->labelTitle = (gcnew System::Windows::Forms::Label());
			this->labelSubTitle = (gcnew System::Windows::Forms::Label());
			this->statusStrip1 = (gcnew System::Windows::Forms::StatusStrip());
			this->toolStripStatusAuthor = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->toolStripStatusVersion = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->textHelp1 = (gcnew System::Windows::Forms::TextBox());
			this->textHelp2 = (gcnew System::Windows::Forms::TextBox());
			this->textHelp3 = (gcnew System::Windows::Forms::TextBox());
			this->textHelp4 = (gcnew System::Windows::Forms::TextBox());
			this->pictureLogo = (gcnew System::Windows::Forms::PictureBox());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numericStartYear))->BeginInit();
			this->statusStrip1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureLogo))->BeginInit();
			this->SuspendLayout();
			// 
			// btnApply
			// 
			this->btnApply->Location = System::Drawing::Point(221, 383);
			this->btnApply->Name = L"btnApply";
			this->btnApply->Size = System::Drawing::Size(95, 78);
			this->btnApply->TabIndex = 3;
			this->btnApply->Text = L"Apply Patch Only";
			this->btnApply->UseVisualStyleBackColor = true;
			this->btnApply->Click += gcnew System::EventHandler(this, &CSD_Form::btnApply_Click);
			// 
			// textOutcome
			// 
			this->textOutcome->Location = System::Drawing::Point(86, 318);
			this->textOutcome->Name = L"textOutcome";
			this->textOutcome->ReadOnly = true;
			this->textOutcome->Size = System::Drawing::Size(230, 20);
			this->textOutcome->TabIndex = 9;
			// 
			// numericStartYear
			// 
			this->numericStartYear->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 14, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->numericStartYear->Location = System::Drawing::Point(214, 344);
			this->numericStartYear->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {9999, 0, 0, 0});
			this->numericStartYear->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) {1900, 0, 0, 0});
			this->numericStartYear->Name = L"numericStartYear";
			this->numericStartYear->Size = System::Drawing::Size(102, 29);
			this->numericStartYear->TabIndex = 0;
			this->numericStartYear->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->numericStartYear->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {1900, 0, 0, 0});
			// 
			// btnWindowed
			// 
			this->btnWindowed->Location = System::Drawing::Point(117, 383);
			this->btnWindowed->Name = L"btnWindowed";
			this->btnWindowed->Size = System::Drawing::Size(95, 78);
			this->btnWindowed->TabIndex = 2;
			this->btnWindowed->Text = L"Launch and Patch EHM\r\n(Windowed)";
			this->btnWindowed->UseVisualStyleBackColor = true;
			this->btnWindowed->Click += gcnew System::EventHandler(this, &CSD_Form::btnWindowed_Click);
			// 
			// btnStandard
			// 
			this->btnStandard->Location = System::Drawing::Point(12, 383);
			this->btnStandard->Name = L"btnStandard";
			this->btnStandard->Size = System::Drawing::Size(95, 78);
			this->btnStandard->TabIndex = 1;
			this->btnStandard->Text = L"Launch and Patch EHM\r\n(Standard)";
			this->btnStandard->UseVisualStyleBackColor = true;
			this->btnStandard->Click += gcnew System::EventHandler(this, &CSD_Form::btnStandard_Click);
			// 
			// textStatus
			// 
			this->textStatus->Location = System::Drawing::Point(12, 220);
			this->textStatus->Multiline = true;
			this->textStatus->Name = L"textStatus";
			this->textStatus->ReadOnly = true;
			this->textStatus->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->textStatus->Size = System::Drawing::Size(304, 92);
			this->textStatus->TabIndex = 7;
			// 
			// textEHMDetails
			// 
			this->textEHMDetails->Location = System::Drawing::Point(86, 194);
			this->textEHMDetails->Name = L"textEHMDetails";
			this->textEHMDetails->ReadOnly = true;
			this->textEHMDetails->Size = System::Drawing::Size(230, 20);
			this->textEHMDetails->TabIndex = 6;
			// 
			// checkAutoExit
			// 
			this->checkAutoExit->AutoSize = true;
			this->checkAutoExit->Checked = true;
			this->checkAutoExit->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkAutoExit->Location = System::Drawing::Point(62, 467);
			this->checkAutoExit->Name = L"checkAutoExit";
			this->checkAutoExit->Size = System::Drawing::Size(204, 17);
			this->checkAutoExit->TabIndex = 4;
			this->checkAutoExit->Text = L"Automatically close this patch window";
			this->checkAutoExit->UseVisualStyleBackColor = true;
			// 
			// labelStatus
			// 
			this->labelStatus->AutoSize = true;
			this->labelStatus->Location = System::Drawing::Point(9, 197);
			this->labelStatus->Name = L"labelStatus";
			this->labelStatus->Size = System::Drawing::Size(67, 13);
			this->labelStatus->TabIndex = 5;
			this->labelStatus->Text = L"EHM Status:";
			// 
			// labelOutcome
			// 
			this->labelOutcome->AutoSize = true;
			this->labelOutcome->Location = System::Drawing::Point(9, 321);
			this->labelOutcome->Name = L"labelOutcome";
			this->labelOutcome->Size = System::Drawing::Size(71, 13);
			this->labelOutcome->TabIndex = 8;
			this->labelOutcome->Text = L"Patch Status:";
			// 
			// labelStartYear
			// 
			this->labelStartYear->AutoSize = true;
			this->labelStartYear->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 14, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->labelStartYear->Location = System::Drawing::Point(8, 346);
			this->labelStartYear->Name = L"labelStartYear";
			this->labelStartYear->Size = System::Drawing::Size(200, 24);
			this->labelStartYear->TabIndex = 17;
			this->labelStartYear->Text = L"Select EHM Start Year:";
			// 
			// linkLabelTBL
			// 
			this->linkLabelTBL->AutoSize = true;
			this->linkLabelTBL->Location = System::Drawing::Point(95, 56);
			this->linkLabelTBL->Name = L"linkLabelTBL";
			this->linkLabelTBL->Size = System::Drawing::Size(208, 13);
			this->linkLabelTBL->TabIndex = 12;
			this->linkLabelTBL->TabStop = true;
			this->linkLabelTBL->Text = L"http://www.ehmtheblueline.com/csdpatch";
			this->linkLabelTBL->LinkClicked += gcnew System::Windows::Forms::LinkLabelLinkClickedEventHandler(this, &CSD_Form::linkLabelTBL_LinkClicked);
			// 
			// labelTitle
			// 
			this->labelTitle->AutoSize = true;
			this->labelTitle->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 14, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->labelTitle->Location = System::Drawing::Point(83, 14);
			this->labelTitle->Name = L"labelTitle";
			this->labelTitle->Size = System::Drawing::Size(233, 24);
			this->labelTitle->TabIndex = 10;
			this->labelTitle->Text = L"Custom Start Date Patch";
			// 
			// labelSubTitle
			// 
			this->labelSubTitle->AutoSize = true;
			this->labelSubTitle->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->labelSubTitle->Location = System::Drawing::Point(95, 39);
			this->labelSubTitle->Name = L"labelSubTitle";
			this->labelSubTitle->Size = System::Drawing::Size(209, 13);
			this->labelSubTitle->TabIndex = 11;
			this->labelSubTitle->Text = L"For Eastside Hockey Manager 2007";
			// 
			// statusStrip1
			// 
			this->statusStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->toolStripStatusAuthor, 
				this->toolStripStatusVersion});
			this->statusStrip1->Location = System::Drawing::Point(0, 487);
			this->statusStrip1->Name = L"statusStrip1";
			this->statusStrip1->Size = System::Drawing::Size(328, 22);
			this->statusStrip1->SizingGrip = false;
			this->statusStrip1->TabIndex = 18;
			this->statusStrip1->Text = L"statusStrip1";
			// 
			// toolStripStatusAuthor
			// 
			this->toolStripStatusAuthor->Name = L"toolStripStatusAuthor";
			this->toolStripStatusAuthor->RightToLeft = System::Windows::Forms::RightToLeft::No;
			this->toolStripStatusAuthor->Size = System::Drawing::Size(87, 17);
			this->toolStripStatusAuthor->Text = L"By Archibalduk";
			this->toolStripStatusAuthor->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
			// 
			// toolStripStatusVersion
			// 
			this->toolStripStatusVersion->Name = L"toolStripStatusVersion";
			this->toolStripStatusVersion->Size = System::Drawing::Size(195, 17);
			this->toolStripStatusVersion->Spring = true;
			this->toolStripStatusVersion->Text = L"Version 1.5";
			this->toolStripStatusVersion->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
			// 
			// textHelp1
			// 
			this->textHelp1->BorderStyle = System::Windows::Forms::BorderStyle::None;
			this->textHelp1->Cursor = System::Windows::Forms::Cursors::Default;
			this->textHelp1->Location = System::Drawing::Point(12, 88);
			this->textHelp1->Multiline = true;
			this->textHelp1->Name = L"textHelp1";
			this->textHelp1->ReadOnly = true;
			this->textHelp1->Size = System::Drawing::Size(304, 30);
			this->textHelp1->TabIndex = 13;
			this->textHelp1->Text = L"Use this patch when you want to start a new game or reload a game where you have " 
				L"not simmed beyond:";
			// 
			// textHelp2
			// 
			this->textHelp2->BorderStyle = System::Windows::Forms::BorderStyle::None;
			this->textHelp2->Cursor = System::Windows::Forms::Cursors::Default;
			this->textHelp2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->textHelp2->Location = System::Drawing::Point(12, 124);
			this->textHelp2->Multiline = true;
			this->textHelp2->Name = L"textHelp2";
			this->textHelp2->ReadOnly = true;
			this->textHelp2->Size = System::Drawing::Size(304, 16);
			this->textHelp2->TabIndex = 14;
			this->textHelp2->Text = L"1 July at the end of season 1 (Russia/Finland)";
			this->textHelp2->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// textHelp3
			// 
			this->textHelp3->BorderStyle = System::Windows::Forms::BorderStyle::None;
			this->textHelp3->Cursor = System::Windows::Forms::Cursors::Default;
			this->textHelp3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->textHelp3->Location = System::Drawing::Point(12, 146);
			this->textHelp3->Multiline = true;
			this->textHelp3->Name = L"textHelp3";
			this->textHelp3->ReadOnly = true;
			this->textHelp3->Size = System::Drawing::Size(304, 16);
			this->textHelp3->TabIndex = 15;
			this->textHelp3->Text = L"1 February of season 1 (all other leagues)";
			this->textHelp3->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// textHelp4
			// 
			this->textHelp4->BorderStyle = System::Windows::Forms::BorderStyle::None;
			this->textHelp4->Cursor = System::Windows::Forms::Cursors::Default;
			this->textHelp4->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->textHelp4->Location = System::Drawing::Point(12, 168);
			this->textHelp4->Multiline = true;
			this->textHelp4->Name = L"textHelp4";
			this->textHelp4->ReadOnly = true;
			this->textHelp4->Size = System::Drawing::Size(304, 20);
			this->textHelp4->TabIndex = 16;
			this->textHelp4->Text = L"DO NOT use the patch if reloading game after 1 July";
			this->textHelp4->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// pictureLogo
			// 
			this->pictureLogo->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"pictureLogo.Image")));
			this->pictureLogo->Location = System::Drawing::Point(12, 12);
			this->pictureLogo->Name = L"pictureLogo";
			this->pictureLogo->Size = System::Drawing::Size(64, 64);
			this->pictureLogo->TabIndex = 19;
			this->pictureLogo->TabStop = false;
			// 
			// CSD_Form
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(328, 509);
			this->Controls->Add(this->pictureLogo);
			this->Controls->Add(this->textHelp4);
			this->Controls->Add(this->textHelp3);
			this->Controls->Add(this->textHelp2);
			this->Controls->Add(this->textHelp1);
			this->Controls->Add(this->statusStrip1);
			this->Controls->Add(this->labelSubTitle);
			this->Controls->Add(this->labelTitle);
			this->Controls->Add(this->linkLabelTBL);
			this->Controls->Add(this->labelStartYear);
			this->Controls->Add(this->labelOutcome);
			this->Controls->Add(this->labelStatus);
			this->Controls->Add(this->checkAutoExit);
			this->Controls->Add(this->textEHMDetails);
			this->Controls->Add(this->textStatus);
			this->Controls->Add(this->btnStandard);
			this->Controls->Add(this->btnWindowed);
			this->Controls->Add(this->numericStartYear);
			this->Controls->Add(this->textOutcome);
			this->Controls->Add(this->btnApply);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->Name = L"CSD_Form";
			this->Text = L"Custom Start Date Patch for EHM 2007";
			this->Load += gcnew System::EventHandler(this, &CSD_Form::CSD_Form_Load);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numericStartYear))->EndInit();
			this->statusStrip1->ResumeLayout(false);
			this->statusStrip1->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureLogo))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	
	private: System::Void CSD_Form_Load(System::Object^  sender, System::EventArgs^  e) {
				 getStartYear();
			 }

	private: System::Void btnApply_Click(System::Object^  sender, System::EventArgs^  e) {
				 resetpatch();
				 detectEHM();
				 memorypatch((int) numericStartYear->Value);
				 patchfinish();
			 }

	private: System::Void btnWindowed_Click(System::Object^  sender, System::EventArgs^  e) {
				 resetpatch();
				 loadEHM(true);
				 detectEHM();
				 memorypatch((int) numericStartYear->Value);
				 patchfinish();
			}
	private: System::Void btnStandard_Click(System::Object^  sender, System::EventArgs^  e) {
				 resetpatch();
				 loadEHM(false);
				 detectEHM();
				 memorypatch((int) numericStartYear->Value);
				 patchfinish();
			}
	private: System::Void linkLabelTBL_LinkClicked(System::Object^  sender, System::Windows::Forms::LinkLabelLinkClickedEventArgs^  e) {
				 System::Diagnostics::Process::Start(L"http://www.ehmtheblueline.com/csdpatch/");
			 }
};
}

