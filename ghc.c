/** Serial: cfe97a42
 * @brief Gh main functions
 * @file ghc.c
 * */
#include "ghcontrol.h"

int main(void)
{
    int logged;
	control_s ctrl = {0};
	reading_s creadings = {0};
	setpoint_s sets = {0};
	alarm_s * arecord;
    arecord = (alarm_s *) calloc(1,sizeof(alarm_s));
    if(arecord == NULL)
    {
        printf("\nCannot allocate memory\n");
        return EXIT_FAILURE;
    }
	sets = GhSetTargets();
	alarmlimit_s alimits = GhSetAlarmLimits();
	GhControllerInit();
	while (1)
	{
		creadings = GhGetReadings();
		logged = GhLogData("ghdata.txt",creadings);
		ctrl = GhSetControls(sets,creadings);
		arecord = GhSetAlarms(arecord,alimits,creadings);
		GhDisplayAll(creadings,sets);
		GhDisplayReadings(creadings);
		GhDisplayTargets(sets);
		GhDisplayControls(ctrl);
		GhDisplayAlarms(arecord);
		GhDelay (GHUPDATE);
	}
	fprintf(stdout,"Press ENTER to continue...");
	getchar();

	return EXIT_FAILURE;
}
