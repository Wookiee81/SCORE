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
void WriteConfigFile(bool InHouseTrack, bool OculusNative, bool RunSetupNext, bool True3D, int PredictionAmmount);

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

void SetupGameConfig()
{
	//Had to seperate this from running the game as it now has a bit more to do.
	string Star_Citizen_Config = HomeDir + ("\\SCORE.ini");
	string Star_Citizen_Game_Config = HomeDir + ("\\CitizenClient\\USER\\game.cfg");
	string config_line;

	// Reading info from the config file, if one does not exist write one and use that instead.
	cout << "Reading SCORE.ini file...";
	ifstream ini_file;
	ini_file.open (Star_Citizen_Config);
	if (ini_file.fail())
	{
		cout << " Failed. \nWriting a default SCORE.ini now and using that instead" << endl;
		WriteConfigFile(1, 0, 1, 1, 25);
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
		getline(ini_file,config_line);
		getline(ini_file,config_line);
		getline(ini_file,config_line);
		getline(ini_file,config_line);
		while (!ini_file.eof())
		{
			getline(ini_file, config_line);
			Config_File << config_line << endl;	
		}
	Config_File.close();
	ini_file.close();
	}
	else
	{
		cout << " Failed. \n-=Make sure SCORE.exe is in your StarCitizen directory=-\n-=Make sure your game.cfg file is not read only=-" << endl;
		Config_File.close();
		ini_file.close();
	}

}

// Argh this gave me the shits something cronic... pulled the code from some site but CreateProcess was just not liking me at all.
void StartStarCitizen()
{
	// Setting up the variables I will need.
	string Star_Citizen_Exec = HomeDir + ("\\CitizenClient\\Bin64\\StarCitizen.exe");
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
void WriteConfigFile(bool InHouseTrack, bool OculusNative, bool RunSetupNext, bool True3D, int PredictionAmmount)
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
		config_file << "SCORE_Version = 0.6.0" << endl;
		config_file << "SCORE_Run_Setup = " << RunSetupNext << endl;
		config_file << "SCORE_In_House_Tracking = " << InHouseTrack << endl;
		config_file << "SCORE_Pred = " << PredictionAmmount << endl;
		config_file << "r_Fullscreen = 1" << endl;
		config_file << "r_Width = 1280" << endl;
		config_file << "r_Height = 1600" << endl;
		config_file << "sys_spec_full = 1" << endl;
		config_file << "r_StereoDevice = 100" << endl;
		config_file << "g_StereoFrameworkEnable = 1" << endl;
		if (True3D)
		{
			config_file << "r_stereomode = 1" << endl;
			config_file << "r_StereoScreenDist = -0.25" << endl;
			config_file << "r_stereoeyeDist = 0.124" << endl;
		}
		else if (!True3D)
		{
			config_file << "r_stereomode = 2" << endl;
			config_file << "r_StereoScreenDist = 0.25" << endl;
			config_file << "r_stereoeyeDist = 0.025" << endl;

		}
		if (!OculusNative)
		{
			config_file << "r_stereoflipeyes = 0" << endl;
			if (InHouseTrack)
			{
				config_file << "sys_CurrentHMDType = 0" << endl;
			}
			else if (!InHouseTrack)
			{
				config_file << "sys_CurrentHMDType = 1" << endl;
			}
			config_file << "r_StereoOutput = 4" << endl;
			
		}
		else if (OculusNative)
		{
			config_file << "r_stereoflipeyes = 1" << endl;
			if (InHouseTrack)
			{
				config_file << "sys_CurrentHMDType = 0" << endl;
			}
			else if (!InHouseTrack)
			{
				config_file << "sys_CurrentHMDType = 1" << endl;
			}
			config_file << "r_StereoOutput = 7" << endl;
		}

		config_file << "cl_fov = 55" << endl;
		config_file << "cl_mp_fov_scalar = 2" << endl;
		config_file << "r_stereostrength = 1.1" << endl;
		config_file << "r_stereoeyeDist = 0.124" << endl;
		config_file << "r_MotionBlurShutterSpeed = 0.0" << endl;
		config_file << "i_mouse_accel=0" << endl;
		config_file << "i_mouse_accel_max=100" << endl;
		config_file << "i_mouse_smooth=0" << endl;
		config_file << "r_VSync = 0" << endl;
		config_file.close();
		cout << "Done" << endl;
	}
}

void Setup_ini_File()
{
	bool InHouse, Native, RunSetup, True3D; 
	int Predictive_Ammount;
	char user_input;

	//Question one SCORE or SC for your head tracking needs, and how much prediction if SCORE.

	cout << "Do you want to use SCORE for predictive head tracking or the current Star Citizen implamentation:" << endl;
	cout << "1) SCORE" << endl;
	cout << "2) Star Citizen" << endl;
	cin >> user_input;

	if (user_input == '1')
	{
		cout << "SCORE it is then" << endl;
		cout << "And how many ms would you like to predict ahead?" << endl;
		cin >> Predictive_Ammount;
		
		InHouse = TRUE;
	}
	else if (user_input == '2')
	{
		cout << "Star Citizen it is then" << endl;
		InHouse = FALSE;
		Predictive_Ammount = 0;
	}
	else
	{
		cout << "I am going to assume you meant SCORE" << endl;
		cout << "And how many ms would you like to predict ahead?" << endl;
		cin >> Predictive_Ammount;
		InHouse = TRUE;
	}

	//Question two, barrel warp? (I recomend no)

	cout << "Do you want to try the native shaders for the oculus rift (WARNING... buggy as hell unless you sellect to use fake 3d next):" << endl;
	cout << "1) I will give it a shot" << endl;
	cout << "2) No thanks it looks like crud" << endl;
	cin >> user_input;

	if (user_input == '1')
	{
		cout << "Good on you for bug testing... there are alot" << endl;
		Native = TRUE;
	}
	else if (user_input == '2')
	{
		cout << "Not perfect but it is much more stable... good choice" << endl;
		Native = FALSE;
	}
	else
	{
		cout << "I am going to assume you want the best looking one... so not native it is" << endl;
		Native = FALSE;
	}

	//Question three, true 3d or not true 3d that is the question.

	cout << "Do you want crappy fake 3d or sexy true 3d:" << endl;
	cout << "You may want fake 3d if using the barel warp" << endl;
	cout << "1) True3D" << endl;
	cout << "2) Fake3D" << endl;
	cin >> user_input;

	if (user_input == '1')
	{
		cout << "True3D it is then" << endl;
		True3D = TRUE;
	}
	else if (user_input == '2')
	{
		cout << "Fake3D it is then... and ewwwwwww" << endl;
		True3D = FALSE;
	}
	else
	{
		if (Native)
		{
		cout << "Since you picked the native shaders I will assume you want fake 3D" << endl;
		True3D = FALSE;
		}
		else if (!Native)
		{
		cout << "Since you did not the native shaders I will assume you want true 3D" << endl;
		True3D = TRUE;
		}
	}

	//Question four, Auto Skip this whole shebang.

	cout << "Do you want to skip any prompts and just use the settings next time you run SCORE:" << endl;
	cout << "You can always change this back in the SCORE.ini later by setting SCORE_Run_Setup to 1" << endl;
	cout << "1) Autorun Star Citizen please" << endl;
	cout << "2) Give me some prompts and options" << endl;
	cin >> user_input;

	if (user_input == '1')
	{
		cout << "Ok autorun enabled for next time" << endl;
		RunSetup = FALSE;
	}
	else if (user_input == '2')
	{
		cout << "You will be prompted for some input next time" << endl;
		RunSetup = TRUE;
	}
	else
	{
		cout << "I am going to assume you want to be prompted next time" << endl;
		RunSetup = TRUE;
	}

	cout << "Well thats all the questions answerd, writing you a new SCORE.ini" << endl;
	WriteConfigFile(InHouse, Native, RunSetup, True3D, Predictive_Ammount);

}

// little program to write a normal game.cfg file
void RestoreGameConfig()
{
	string Star_Citizen_Game_Config = HomeDir + ("\\CitizenClient\\USER\\game.cfg");
	ofstream config_file;
	config_file.open (Star_Citizen_Game_Config);
		if (config_file.fail())
	{
		cout << " Failed. \n I am not sure this can even happen... /nIf you see this msg please post as much in the forum thread." << endl << endl;
		config_file.close();
	}
	else
	{
		config_file << "-- [Game-Configuration]" << endl;
		config_file << "-- Attention: This file is re-generated by the system! Editing is not recommended!" << endl << endl;
		config_file << "r_stereoDevice = 100" << endl;
		config_file << "r_stereoMode = 0" << endl;
		config_file << "r_stereoOutput = 0" << endl;
		config_file << "sys_spec = 3" << endl;
		config_file.close();
	}
}

// This part generates the outputs from the oculus to be sent to the mouse. and also calls the other subroutines.
void Output()
{

	char Customise;
	bool autoskip, InHouse;
	int prediction_ammount;

	cout << "-= Freeboot's Star Citizen Oculus Rift Emulator Ver 0.6.0 =-" << endl;

	//Need to open and read the ini file to check if it exists, is up to date, has the skip flag checked.
	string Star_Citizen_Config = HomeDir + ("\\SCORE.ini");
	string config_line;
	// Reading info from the config file, if one does not exist write one and use that instead.
	cout << "Reading SCORE.ini file..." << endl;
	ifstream ini_file;
	ini_file.open (Star_Citizen_Config);
	if (ini_file.fail())
	{
		cout << " Failed. \nWriting a default SCORE.ini now and using that instead" << endl;
		WriteConfigFile(1, 0, 1, 1, 25);
		ini_file.open (Star_Citizen_Config);
	}
	getline(ini_file, config_line);
	if (config_line != "SCORE_Version = 0.6.0")
	{
		cout << "SCORE.ini version mismatch, updating ini file with default." << endl;
		ini_file.close();
		WriteConfigFile(1, 0, 1, 1, 25);
		ini_file.open(Star_Citizen_Config);
		getline(ini_file, config_line);
	}

	getline(ini_file, config_line);
	config_line.erase(0, 17);
	if (atof(config_line.c_str()) == 0) autoskip = TRUE;
	else autoskip = FALSE;
	if (autoskip)

	{
		cout << "Auto running with no prompts for user input" << endl;
	}
	else if (!autoskip)
	{
		cout << "Do you want to customize your options?" << endl;
		cout << "Y/N " << endl;
		cin >> Customise;
		if ((Customise == 'y') || (Customise == 'Y'))
		{
			ini_file.close();
			Setup_ini_File();
			ini_file.open(Star_Citizen_Config);
			getline(ini_file, config_line);
			getline(ini_file, config_line);
		}
	}		

	getline(ini_file, config_line);
	config_line.erase(0, 25);
	if (atof(config_line.c_str()) != 1) InHouse = FALSE;
	else if (atof(config_line.c_str()) == 1)  InHouse = TRUE;

	getline(ini_file, config_line);
	config_line.erase(0, 12);
	prediction_ammount = atof(config_line.c_str());
	ini_file.close();

	int NewX, NewY, OldX = 0, OldY = 0, DeltaX = 0, DeltaY = 0;
	const int multi = 1044;
	float yaw, pitch, roll;
	float PredMS = (prediction_ammount / 1000);

	//commenting this out for testing of the variables.
	SetupGameConfig();
	StartStarCitizen();
	cout << "--------------------------------------------------------" << endl;
	cout << "Running with your config file now:" << endl;
	cout << "Press enter to stop and restor a normal game.cfg" << endl;

	if (InHouse)
	{
		//cout << "InHouse flag" << endl;
		Init();
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

			if (_kbhit())
			{
				//Need to swap back the game config file here.
				RestoreGameConfig();
				exit(0);
			}
		}
	}
	else if (!InHouse)
	{
		//cout << "OutHouse flag" << endl;
		while (!(_kbhit()))
		{
			Sleep(1);
		} 

		if (_kbhit())
		{
			//Need to swap back the game config file here.
			RestoreGameConfig();
			exit(0);
		}
	}
}


// This tiny thing is it... Yep underwhelming.
int main()
{
	Output();
	Clear();
}
