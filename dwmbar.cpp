#include "dwmbar.h"

int main (int argc, char **argv)

{

	std::string bar;
	char args [7];

	std::cout << argc;

	if (argc > 1)

	{

		strcpy (args, argv[1]);
	
		if (strcmp (args, "-print") == 0)

		{

			while (true)

			{

				std::cout << "\r" << ParseDISPLAY ().c_str ();
				std::this_thread::sleep_for (std::chrono::milliseconds (UPDATE_DELAY));

			}

		}

		else

		{

			while (true)

			{

				SetRootName (ParseDISPLAY ().c_str ());
				std::this_thread::sleep_for (std::chrono::milliseconds (UPDATE_DELAY));

			}

		}

	}

	else

	{


		while (true)

		{

			SetRootName (ParseDISPLAY ().c_str ());
			//std::cout << ParseDISPLAY ().c_str () << std::endl;

			std::this_thread::sleep_for (std::chrono::milliseconds (UPDATE_DELAY));

		}

	}

	return 0;

}

std::string BATTERY_charge ()

{

	std::string charge;

	std::ifstream capacity;
	capacity.open ("/sys/class/power_supply/BAT0/capacity");

	if (capacity.is_open ())
		std::getline (capacity, charge);

	else
		charge = "?";

	capacity.close ();

	//while (charge.size () < 3 and charge != "?")
	//	charge = "-" + charge;

	return charge;

}

std::string BATTERY_state ()

{

	std::string state;

	std::ifstream status;
	status.open ("/sys/class/power_supply/BAT0/status");

	if (status.is_open ())
		std::getline (status, state);

	else
		state = "?";

	status.close ();

	return state;

}

std::string DATE_TIME ()

{

	std::string out;

	char date[64];
	time_t rawtime;
	time (&rawtime);
	struct tm *info;

	info = localtime (&rawtime);
	strftime (date, 64, DATE_DISPLAY, info);
	out = date;

	return out;

}

std::string MEM_use ()

{

	struct sysinfo sysinf;

	sysinfo (&sysinf);
	unsigned long long int RAM_use = sysinf.totalram - sysinf.freeram;
	long long int RAM_load = RAM_use / 1024;	//convert to kB

	return std::to_string (RAM_load);

}

std::string MEM_load ()

{

	std::string out;

	struct sysinfo sysinf;
	sysinfo (&sysinf);

	unsigned long long int RAM_use = sysinf.totalram - sysinf.freeram, RAM_total = sysinf.totalram;

	double RAM_percent = (((double) RAM_use / 1048576) / ((double)RAM_total / 1048576)) * 100;

	int RAM_percent_round = round (RAM_percent);

	out = std::to_string (RAM_percent_round);

	//while (out.size () < 3)
	//	out = "-" + out;

	return out;

}

std::string CPU ()

{

	std::string out;
	double CPU_percent;
	int CPU_percent_trunc;
	double CPU_total, CPU_work;

	double a[9], b[9];
	a[0] = 0;
	b[0] = 0;

	std::string fromFile;

	std::ifstream cpufile;
	cpufile.open ("/proc/stat");

	if (cpufile.is_open ())

	{

		for (int i = 0; i < 9; i ++)

		{

			cpufile >> fromFile;

			if (i != 0)
				a[i] = atof (fromFile.c_str ());

		}

		cpufile.close ();

		CPU_total = a[1] + a[2] + a[3] + a[4] + a[5] + a[6] + a[7] + a[8];

	}

	else

	{

		CPU_total = 0;

	}

	cpufile.open ("/proc/stat");

	if (cpufile.is_open ())

	{

		for (int i = 0; i < 9; i ++)

		{

			cpufile >> fromFile;

			if (i != 0)
				b[i] = atof (fromFile.c_str ());

		}

		cpufile.close ();

		CPU_work = b[1] + b[2] + b[3];

	}

	else

	{

		CPU_work = 0;

	}

	CPU_percent = ((CPU_work - lastCPU[0]) / (CPU_total - lastCPU[1])) * 100;
	CPU_percent_trunc = (int) CPU_percent;

	lastCPU[0] = CPU_work;
	lastCPU[1] = CPU_total;
	out = std::to_string (CPU_percent_trunc);

	//while (out.size () < 3)
	//	out = "-" + out;

	return out;

}

std::string PROCESSES ()

{

	std::string running;

	std::ifstream procs;
	procs.open ("/proc/stat");
	if (procs.is_open ())

	{
		for (int i = 0; i < 13; i++)
			std::getline (procs, running);

		running.erase (0, 10);

	}
	else
		running = "?";

	procs.close ();

	//while (running.size () < 7 and running != "?")
	//	running = "-" + running;

	return running;

}

void SetRootName (const char *name)

{

	Display *display;
	display = XOpenDisplay (NULL);
	XStoreName (display, DefaultRootWindow (display), name);
	XSync (display, 0);
	XCloseDisplay (display);

}

std::string ParseDISPLAY ()

{

	std::string out = "", functionName = "";
	char a;
	std::istringstream parse (DISPLAY);

	while (parse.get(a))

	{

		if (a == '`')

		{

			while (parse.get(a))

			{
				if (a == '`')
					break;

				functionName.append (1, a);

			}

			if (functionName == "BATTERY_charge")
				out = out + BATTERY_charge ();

			else if (functionName == "BATTERY_state")
				out = out + BATTERY_state ();

			else if (functionName == "DATE_TIME")
				out = out + DATE_TIME ();

			else if (functionName == "MEM")
				out = out + MEM_use ();

			else if (functionName == "MEM_load")
				out = out + MEM_load ();

			else if (functionName == "CPU")
				out = out + CPU ();

			else if (functionName == "PROCESSES")
				out = out + PROCESSES ();

			else {}

			functionName.clear ();

		}

		else
			out = out + a;

	}

	return out;
}
