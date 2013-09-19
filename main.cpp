#include <iostream>
#include <fstream>
#include <conio.h>
#include <string>
#include <algorithm>
#include "Windows.h"
#include "OVR.h"

using namespace OVR;
using namespace std;

Ptr<DeviceManager>	pManager;
Ptr<HMDDevice>		pHMD;
Ptr<SensorDevice>	pSensor;
SensorFusion		FusionResult;
HMDInfo			Info;
bool			InfoLoaded;
void WriteConfigFile();

// Fairly sure this is bad practice but fuck it at this point I don't care any more.
const string TempDir = _pgmptr;
const string HomeDir = TempDir.substr(0, TempDir.find_last_of("\\/"));

// This will init the oculus and the directory for the star citizen executable.
void Init()
{
	System::Init();

	pManager = *DeviceManager::Create();

	pHMD = *pManager->EnumerateDevices<HMDDevice>().CreateDevice();

	if (pHMD)
        {
           InfoLoaded = pHMD->GetDeviceInfo(&Info);
	   pSensor = *pHMD->GetSensor();
	}
	else
	{
	   pSensor = *pManager->EnumerateDevices<SensorDevice>().CreateDevice();
	}

	if (pSensor)
	{
	   FusionResult.AttachToSensor(pSensor);
	}
}

// This I assume clears the connection to the oculus... to be honest this is cyberealitys doing not mine.
void Clear()
{
	pSensor.Clear();
        pHMD.Clear();
	pManager.Clear();

	System::Destroy();
}

// Push the translated oculus info to the mouse.
void SendMouseTranslate(const int x, const int y)
{
	INPUT i;

	i.type = INPUT_MOUSE;
	i.mi.dx = x;
	i.mi.dy = y;
	i.mi.mouseData = 0;
	i.mi.dwFlags = (MOUSEEVENTF_MOVE || MOUSEEVENTF_ABSOLUTE);
	i.mi.time = 0;
	i.mi.dwExtraInfo = GetMessageExtraInfo();
	SendInput(1, &i, sizeof(i));
}

// Argh this gave me the shits something cronic... pulled the code from some site but CreateProcess was just not liking me at all.
void StartStarCitizen()
{
	// Setting up the variables I will need.
	string Star_Citizen_Exec = HomeDir + ("\\CitizenClient\\Bin64\\StarCitizen.exe");
	string Star_Citizen_Config = HomeDir + ("\\SCORE.ini");
	string Star_Citizen_Game_Config = HomeDir + ("\\CitizenClient\\USER\\game.cfg");
	string config_line;
	int SizeOfExecArray = Star_Citizen_Exec.size();
	WCHAR Exec[MAX_PATH] = { };

	// Convert the executable path into something compatable with the CreateProcess command.
	for (int a=0;a<=SizeOfExecArray;a++)
    {
        Exec[a]=Star_Citizen_Exec[a];
		if (a == SizeOfExecArray)
		{
			Exec[a]=NULL;
		}
    }

	// Next setting up the boring bits to actually start an external program.
	PROCESS_INFORMATION ProcessInfo; //This is what we get as an [out] parameter
	STARTUPINFO StartupInfo; //This is an [in] parameter
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));
	StartupInfo.cb = sizeof StartupInfo ; //Only compulsory field

	// Reading info from the config file, if one does not exist write one and use that instead.
	cout << "Reading SCORE.ini file...";
	ifstream ini_file;
	ini_file.open (Star_Citizen_Config);
	if (ini_file.fail())
	{
		cout << " Failed. \n-=Writing a new SCORE.ini now and using that instead=-" << endl;
		WriteConfigFile();
		ini_file.open (Star_Citizen_Config);
	}
	else
	{
		cout << " Success." << endl;
	}
	ofstream Config_File;
	cout << "Writting a new game.cfg file...";
	Config_File.open(Star_Citizen_Game_Config);
	if (!Config_File.fail())
	{
		while (!ini_file.eof())
		{
			getline(ini_file, config_line);
			Config_File << config_line << endl;	
		}
	Config_File.close();
	ini_file.close();
	cout << " Success." << endl;
	}
	else
	{
		cout << " Failed. \n-=Make sure SCORE.exe is in your StarCitizen directory=-\n-=Make sure your game.cfg file is not read only=-" << endl;
		Config_File.close();
		ini_file.close();
	}

	// Now lets pull all this together and kick of the program.
	cout << "Starting program...";
	//string Arguments = "+r_motionblur=0";
	if (!CreateProcess((LPCWSTR)Exec, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInfo))
	{
		cout << " Failed.\n-=Put SCORE.exe in your StarCitizen directory=-" << endl;
	}
	else
	{
		cout << " Success." << endl;
	}
}

// Does what is says on the box.
void WriteConfigFile()
{
	cout << "Writting a new SCORE.ini file...";
	string Star_Citizen_Config = HomeDir + ("\\SCORE.ini");
	ofstream config_file;
	config_file.open (Star_Citizen_Config);
	if (config_file.fail())
	{
		cout << " Failed. \n-=Make sure the file does not exist or is not read only=-" << endl << endl;
		config_file.close();
	}
	else
	{
	config_file << "r_Fullscreen = 1" << endl;
	config_file << "r_Width = 1280" << endl;
	config_file << "r_Height = 1600" << endl;
	config_file << "sys_spec = 1" << endl;
	config_file << "r_StereoDevice = 100" << endl;
	config_file << "r_StereoScreenDist = -0.25" << endl;
	config_file << "r_stereomode = 1" << endl;
	config_file << "r_stereoflipeyes = 0" << endl;
	config_file << "sys_CurrentHMDType = 1" << endl;
	config_file << "r_StereoOutput = 4" << endl;
	config_file << "cl_fov = 55" << endl;
	config_file << "cl_mp_fov_scalar = 2" << endl;
	config_file << "r_stereostrength = 8" << endl;
	config_file << "r_stereoeyeDist = 0.124" << endl;
	config_file << "r_MotionBlurShutterSpeed = 0.0" << endl;
	config_file << "i_mouse_accel=0" << endl;
	config_file << "i_mouse_accel_max=100" << endl;
	config_file << "i_mouse_smooth=0" << endl;
	config_file << "r_VSync = 0";
	config_file.close();
	cout << " Success." << endl;
	}
}

// This part generates the outputs from the oculus to be sent to the mouse. and also calls the other subroutines.
void Output(const float Pred)
{
	cout << "- Freeboot's Star Citizen Oculus Rift Emulator V0.5.1  -" << endl;
	cout << "Press ENTER to do all that stuff to good make game look." << endl;
	cout << "--------------------------------------------------------" << endl;	
	cout << "You have selected: " << Pred << " ms of prediction." << endl;

	cin.get();

	cout << "--------------------------------------------------------" << endl;
	int NewX, NewY, OldX = 0, OldY = 0, DeltaX = 0, DeltaY = 0;
	const int multi = 1044;
	float yaw, pitch, roll;
	float PredMS = (Pred / 1000);

	StartStarCitizen();
	cout << "--------------------------------------------------------" << endl;
	cout << "Enjoy, close this AFTER you are done by presing ESC." << endl;

	while(pSensor)
	{
		Quatf shift = FusionResult.GetPredictedOrientation(PredMS);

		shift.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch, &roll);

		NewX = int(yaw * multi);
		NewY = int(pitch * multi);
		DeltaX = OldX - NewX;
		DeltaY = OldY - NewY;

		if (DeltaY != 0 || DeltaX !=0 )
		{
			SendMouseTranslate(DeltaX, DeltaY);
		OldX = NewX;
		OldY = NewY;
		}

		Sleep(1);

		if (_kbhit()) exit(0);
	}
}


// This tiny thing is it... Yep underwhelming.
int main(int argc,char *argv[])
{
	float Pred;
	if( argc > 1 ){
		Pred = atof(argv[1]);
	}
	else{
		Pred = 0;
	}
	Init();
	Output(Pred);
	Clear();
}
