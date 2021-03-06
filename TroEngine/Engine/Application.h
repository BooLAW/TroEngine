#pragma once

#include <list>
#include "Globals.h"
#include "Timer.h"
#include "PerfTimer.h"
#include "Module.h"
#include <time.h>
#include <vector>
#include "MathGeoLibFwd.h"
#include "MathGeoLib.h"

class Module;
class ModuleWindow;
class ModuleInput;
class ModuleAudio;
class EditorScene;
class ModuleRenderer3D;
class ModuleCamera3D;
class ModuleGUI;
class EditorConsole;
class EditorGUI;

class Application
{
public:
	ModuleWindow* window;
	ModuleInput* input;
	ModuleAudio* audio;
	EditorScene* scene_intro;
	ModuleRenderer3D* renderer3D;
	ModuleCamera3D* camera;
	ModuleGUI* gui;
	EditorConsole* console;
	EditorGUI* editor_gui;

private:

	std::list<Module*>	list_modules;

	bool				close_app = false;

	LCG					random_generator;

	std::string organization;

	//Congiguration
	char*				new_title;
	char*				new_org;
	int					new_fps = 0;
	std::vector<float>	fps_log;
	std::vector<float>	ms_log;
	std::vector<float>	mem_log;
	PerfTimer			logic_timer;

	// Engine debug info
	int					capped_ms = -1;
	PerfTimer			ptimer;
	Uint64				frame_count = 0;
	Timer				startup_time;
	Timer				frame_time;
	Timer				last_sec_frame_time;
	Uint32				last_sec_frame_count = 0;
	Uint32				prev_last_sec_frame_count = 0;
	float				dt = 0.0f;

public:

	Application();
	~Application();

	bool Init();
	update_status Update();
	bool CleanUp();

	void CloseApp();

	float RandFloat();
	int RandRange(int min, int max);

	void ConfigGUI();
	void HardwareConfig();

	void OpenWebPage(const char* url);

private:

	void AddModule(Module* mod);
	void PrepareUpdate();
	void FinishUpdate();

	void CapFPS(float fps);
	void FrameRateCalculations();
};

extern Application* App;