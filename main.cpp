#include "OVR.h"
#include <iostream>
#include <fstream>
#include <conio.h>
#include <string>
#include "Windows.h"

using namespace OVR;
using namespace std;

Ptr<DeviceManager>	pManager;
Ptr<HMDDevice>		pHMD;
Ptr<SensorDevice>	pSensor;
SensorFusion		FusionResult;
HMDInfo			Info;
bool			InfoLoaded;

// Fairly sure this is bad practice but fuck it at this point I don't care any more.
string TempDir = _pgmptr;

string HomeDir = TempDir.substr(0, TempDir.find_last_of("\\/"));
string Star_Citizen_Exec = HomeDir + ("\\CitizenClient\\Bin64\\StarCitizen.exe");
string Star_Citizen_Config = HomeDir + ("\\CitizenClient\\USER\\game.cfg");

// Had this problem stuck in my head for 2 days... this is my solution, pass the directory to a WCHAR array... There is probably an easier way but this is the first thing that worked for me.
int SizeOfCharArray = Star_Citizen_Exec.size();
WCHAR Exec[MAX_PATH] = { };

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
	for (int a=0;a<=SizeOfCharArray;a++)
    {
        Exec[a]=Star_Citizen_Exec[a];
		if (a == SizeOfCharArray)
		{
			Exec[a]=NULL;
		}
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

// Arg this gave me the shits something cronic... pulled the code from some site but CreateProcess was just not liking me at all.
void StartStarCitizen()
{
	PROCESS_INFORMATION ProcessInfo; //This is what we get as an [out] parameter
	STARTUPINFO StartupInfo; //This is an [in] parameter
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));
	StartupInfo.cb = sizeof StartupInfo ; //Only compulsory field
	CreateProcess((LPCWSTR)Exec, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInfo);
}

// Does what is says on the box.
void WriteConfigFile()
{
	ofstream config_file;
	config_file.open (Star_Citizen_Config);
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
}

// This part generates the outputs from the oculus to be sent to the mouse. and also calls the other subroutines.
void Output(const float Pred)
{
	cout << "------ Freeboot's Star Citizen Oculus Interpreter ------" << endl;
	cout << "Press ENTER to do all that stuff to good make game look." << endl;
	cout << "--------------------------------------------------------" << endl;	
	cout << "You have selected: " << Pred << "ms of prediction." << endl;

	cin.get();

	cout << "--------------------------------------------------------" << endl;
	cout << "Initialising oculus rift settings..." << endl;
	int NewX, NewY, OldX = 0, OldY = 0, DeltaX = 0, DeltaY = 0;
	const int multi = 1044;
	float yaw, pitch, roll;
	float PredMS = (Pred / 1000);
	cout << "--------------------------------------------------------" << endl;
	cout << "Writting a new configeration file at location here:" << endl << Star_Citizen_Config << endl;
	WriteConfigFile();
	cout << "--------------------------------------------------------" << endl;
	cout << "Starting program located here:" << endl << Star_Citizen_Exec << endl;
	StartStarCitizen();
	cout << "--------------------------------------------------------" << endl;
	cout << "Enjoy, close this after you are done by presing ESC." << endl;

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

		//Sleep(1);

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
