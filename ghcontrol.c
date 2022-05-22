/** @brief Gh control functions
*   @file ghcontrol.c
*/
#include "ghcontrol.h"

// Alarm Message Array
const char alarmnames[NALARMS][ALARMNMSZ] = {"No Alarms","High Temperature","Low Temperature","High Humidity","Low Humidity","High Pressure","Low Pressure"};


//Function Definitions
/** @brief Prints Gh Controller Title
 *  @version 19FEB2021
 *  @author Jakob Wood
 *  @param sname string with operator's name
 *  @return void
*/
void GhDisplayHeader(const char * sname)
{
	fprintf(stdout,"%s's CENG153 Greenhouse Controller\n",sname);
}

/** @brief Retrieves Gh Serial Number
 *  @version 19FEB2021
 *  @author Jakob Wood
 *  @param void
 *  @return uint64_t
*/
uint64_t GhGetSerial(void)
{
	static uint64_t serial = 0;
	FILE * fp;
	char buf[SYSINFOBUFSZ];
	char searchstring[] = SEARCHSTR;
	fp = fopen ("/proc/cpuinfo", "r");
	if (fp != NULL)
	{
		while (fgets(buf, sizeof(buf), fp) != NULL)
		{
			if (!strncasecmp(searchstring, buf, strlen(searchstring)))
			{
				sscanf(buf+strlen(searchstring), "%Lx", &serial);
			}
		}
		fclose(fp);
	}
     if(serial==0)
     {
         system("uname -a");
         system("ls --fu /usr/lib/codeblocks | grep -Po '\\.\\K[^ ]+'>stamp.txt");
         fp = fopen ("stamp.txt", "r");
         if (fp != NULL)
         {
             while (fgets(buf, sizeof(buf), fp) != NULL)
             {
                sscanf(buf, "%Lx", &serial);
             }
             fclose(fp);
        }
     }
	return serial;
}

/** @brief Retrieves Random Number
 *  @version 19FEB2021
 *  @author Jakob Wood
 *  @param int as range of numbers
 *  @return int
*/
int GhGetRandom(int range)
{
	return rand() % range;
}

/** @brief Delays Program/Functions
 *  @version 19FEB2021
 *  @author Jakob Wood
 *  @param int as milliseconds
 *  @return void
*/
void GhDelay(int milliseconds)
{
	long wait;
	clock_t now,start;

	wait = milliseconds*(CLOCKS_PER_SEC/1000);
	start = clock();
	now = start;
	while( (now-start) < wait )
	{
		now = clock();
	}
}

/** @brief Calls srand, SetTargets, and DisplayHeader functions
 *  @version 19FEB2021
 *  @author Jakob Wood
 *  @param void
 *  @return void
*/
void GhControllerInit(void)
{
	srand((unsigned) time(NULL));
	GhDisplayHeader("Jakob Wood");
#if SENSEHAT
    ShInit();
#endif
}

/** @brief Prints Heater/Humidifier Controls
 *  @version 30MAR2021
 *  @author Jakob Wood
 *  @param object of controls type
 *  @return void
*/
void GhDisplayControls(control_s ctrl)
{
	fprintf(stdout,"Controls\tHeater: %d\tHumidifier: %d\n",ctrl);
}

/** @brief Prints Readings
 *  @version 30MAR2021
 *  @author Jakob Wood
 *  @param object of readings data
 *  @return void
*/
void GhDisplayReadings(reading_s rdata)
{
    fprintf(stdout,"\nUnit: %LX %sReadings\tT: %4.1lfC\tH: %4.1lf%%\tP: %6.1lfmB\n",GhGetSerial(),ctime(&rdata.rtime),rdata.temperature,rdata.humidity,rdata.pressure);
}


/** @brief Prints Gh Target Data
 *  @version 30MAR2021
 *  @author Jakob Wood
 *  @param void
 *  @return void
*/
void GhDisplayTargets(setpoint_s spts)
{
	fprintf(stdout,"Target Data\tT: %.1lfC\tH: %.1lf%\n",spts.temperature,spts.humidity);
}

/** @brief Sets Heater/Humidifier Controls
 *  @version 07APRIL2021
 *  @author Jakob Wood
 *  @param object of setpoints (targets) data
 *  @param object of readings data
 *  @return void
*/
control_s GhSetControls(setpoint_s target,reading_s rdata)
{
    control_s cset = {0};

    if(rdata.temperature<target.temperature)
    {
        cset.heater = ON;
    }
    else
    {
        cset.heater = OFF;
    }

    if(rdata.humidity<target.humidity)
    {
        cset.humidifier = ON;
    }
    else
    {
        cset.humidifier = OFF;
    }
    return cset;
}

/** @brief Sets Gh Targets
 *  @version 07APRIL2021
 *  @author Jakob Wood
 *  @param void
 *  @return object of setpoints (targets) type
*/
setpoint_s GhSetTargets(void)
{
    setpoint_s cpoints = {0};

    cpoints = GhRetrieveSetpoints("setpoints.dat");
    if(cpoints.temperature == 0)
    {
        cpoints.temperature=STEMP;
        cpoints.humidity=SHUMID;
        GhSaveSetpoints("setpoints.dat",cpoints);
    }
    return cpoints;
}

/** @brief Retrieves/Simulates Humidity
 *  @version 30MAR2021
 *  @author Jakob Wood
 *  @param void
 *  @return double
*/
double GhGetHumidity(void)
{
#if SIMHUMIDITY
	return GhGetRandom(USHUMID-LSHUMID)+LSHUMID;
#else
	ht221sData_s ct = {0};
	ct = ShGetHT221SData();
	return ct.humidity;
#endif
}

/** @brief Retrieves/Simulates Pressure
 *  @version 30MAR2021
 *  @author Jakob Wood
 *  @param void
 *  @return double
*/
double GhGetPressure(void)
{
#if SIMPRESSURE
	return GhGetRandom(USPRESS-LSPRESS)+LSPRESS;
#else
	lps25hData_s ct = {0};
	ct = ShGetLPS25HData();
	return ct.pressure;
#endif
}

/** @brief Retrieves/Simulates Temperature
 *  @version 30MAR2021
 *  @author Jakob Wood
 *  @param void
 *  @return double
*/
double GhGetTemperature(void)
{
#if SIMTEMPERATURE
	return GhGetRandom(USTEMP-LSTEMP)+LSTEMP;
#else
	ht221sData_s ct = {0};
	ct = ShGetHT221SData();
    return ct.temperature;
#endif
}


/** @brief Retrieves Readings
 *  @version 07APRIL2021
 *  @author Jakob Wood
 *  @param void
 *  @return object of readings type
*/
reading_s GhGetReadings(void)
{
    reading_s now = {0};
    now.rtime = time(NULL);
    now.temperature = GhGetTemperature();
    now.humidity = GhGetHumidity();
    now.pressure = GhGetPressure();
	return now;
}

/** @brief Logs Gh Data
 *  @version 07APRIL2021
 *  @author Jakob Wood
 *  @param fname pointer to file name
 *  @param object of readings data
 *  @return int
*/
int GhLogData(char * fname, reading_s ghdata)
{
    FILE *fp;
    char ltime[CTIMESTRSZ];
    fp = fopen(fname,"a");
    if(fp == NULL)
    {
        return 0;
    }
    strcpy(ltime, ctime(&ghdata.rtime));
    ltime[3] = ',';
    ltime[7] = ',';
    ltime[10] = ',';
    ltime[19] = ',';
    fprintf(fp, "\n%.24s,%5.1lf,%5.1lf,%6.1lf",ltime,ghdata.temperature,ghdata.humidity,ghdata.pressure);
    fclose(fp);
    return 1;

}

/** @brief Saves Target Data
 *  @version 07APRIL2021
 *  @author Jakob Wood
 *  @param fname pointer to file name
 *  @param object of setpoint data
 *  @return int
*/
int GhSaveSetpoints(char * fname, setpoint_s spts)
{
    FILE *fp;
    fp = fopen(fname, "w");
    if(fp == NULL)
    {
        return 0;
    }
    else
    {
        fwrite(&spts,sizeof(spts),1,fp);
        fclose(fp);
        return 1;
    }
}

/** @brief Retreives Target Data
 *  @version 07APRIL2021
 *  @author Jakob Wood
 *  @param fname pointer to file name
 *  @return object of setpoints type
*/
setpoint_s GhRetrieveSetpoints(char * fname)
{
    setpoint_s spts = {0.0};
    FILE *fp;
    fp = fopen(fname, "r");
    if(fp == NULL)
    {
        return spts;
    }
    else
    {
        fread(&spts,sizeof(spts),1,fp);
        fclose(fp);
        return spts;
    }
}

/** @brief Displays all readings and setpoints
 *  @version 07APRIL2021
 *  @author Jakob Wood
 *  @param object of readings data
 *  @param object of setpoint data
 *  @return void
*/
void GhDisplayAll(reading_s rd,setpoint_s sd)
{
    int rv;
    int sv;
    int avh;
    int avl;
    struct fbpixel pxc = {0};

    ShClearMatrix();

    rv = (8.0 * (((rd.temperature-LSTEMP) / (USTEMP-LSTEMP))+0.05))-1.0;
    sv = (8.0 * (((sd.temperature-LSTEMP) / (USTEMP-LSTEMP))+0.05))-1.0;
    pxc.red = 0x00;
    pxc.green = 0xFF;
    pxc.blue = 0x00;
    ShSetVerticalBar(TBAR,pxc,rv);
    pxc.red = 0xF0;
    pxc.green = 0x0F;
    pxc.blue = 0xF0;
    ShSetPixel(TBAR,sv,pxc);

    rv = (8.0 * (((rd.humidity-LSHUMID) / (USHUMID-LSHUMID))+0.05))-1.0;
    sv = (8.0 * (((sd.humidity-LSHUMID) / (USHUMID-LSHUMID))+0.05))-1.0;
    pxc.red = 0x00;
    pxc.green = 0xFF;
    pxc.blue = 0x00;
    ShSetVerticalBar(HBAR,pxc,rv);
    pxc.red = 0xF0;
    pxc.green = 0x0F;
    pxc.blue = 0xF0;
    ShSetPixel(HBAR,sv,pxc);

    rv = (8.0 * (((rd.pressure-LSPRESS) / (USPRESS-LSPRESS))+0.05))-1.0;
    pxc.red = 0x00;
    pxc.green = 0xFF;
    pxc.blue = 0x00;
    ShSetVerticalBar(PBAR,pxc,rv);
}

/** @brief Sets Limits for Alarms
 *  @version 07APRIL2021
 *  @author Jakob Wood
 *  @param void
 *  @return object of alarm limits type
*/
alarmlimit_s GhSetAlarmLimits(void)
{
    alarmlimit_s calarm;
    calarm.hight = UPPERATEMP;
    calarm.lowt = LOWERATEMP;
    calarm.highh = UPPERAHUMID;
    calarm.lowh = LOWERAHUMID;
    calarm.highp = UPPERAPRESS;
    calarm.lowp = LOWERAPRESS;
    return calarm;
}

/** @brief Sets Alarms
 *  @version 13APRIL2021
 *  @author Jakob Wood
 *  @param pointer to alarm_s type
 *  @param object of alarm limits type
 *  @param object of readings type
 *  @return void
*/
alarm_s * GhSetAlarms(alarm_s * head,alarmlimit_s alarmpt,reading_s rdata)
{
    if (rdata.temperature >= alarmpt.hight)
    {
        GhSetOneAlarm(HTEMP,rdata.rtime,rdata.temperature,head);
    }
    else
    {
        head = GhClearOneAlarm(HTEMP,head);
    }
    return head;
    if (rdata.pressure >= alarmpt.highp)
    {
        GhSetOneAlarm(HPRESS,rdata.rtime,rdata.pressure,head);
    }
    else
    {
        head = GhClearOneAlarm(HPRESS,head);
    }
    if (rdata.humidity >= alarmpt.highh)
    {
        GhSetOneAlarm(HHUMID,rdata.rtime,rdata.humidity,head);
    }
    else
    {
        head = GhClearOneAlarm(HHUMID,head);
    }
        if (rdata.temperature <= alarmpt.lowt)
    {
        GhSetOneAlarm(LTEMP,rdata.rtime,rdata.temperature,head);
    }
    else
    {
        head = GhClearOneAlarm(LTEMP,head);
    }
    if (rdata.pressure <= alarmpt.lowp)
    {
        GhSetOneAlarm(LPRESS,rdata.rtime,rdata.pressure,head);
    }
    else
    {
        head = GhClearOneAlarm(LPRESS,head);
    }
    if (rdata.humidity <= alarmpt.lowh)
    {
        GhSetOneAlarm(LHUMID,rdata.rtime,rdata.humidity,head);
    }
    else
    {
        head = GhClearOneAlarm(LHUMID,head);
    }
}

/** @brief Displays Alarms
 *  @version 13APRIL2021
 *  @author Jakob Wood
 *  @param object of alarm type
 *  @return void
*/
void GhDisplayAlarms(alarm_s * head)
{
    alarm_s *cur;
    cur = head;
    fprintf(stdout,"\nAlarms\n");
    while (cur != NULL)
    {
        if (cur->code == HTEMP)
        {
            fprintf(stdout,"%s Alarm on %s",alarmnames[HTEMP],ctime(&cur->atime));
            cur = cur->next;
        }
        else if (cur->code == LTEMP)
        {
            fprintf(stdout,"%s Alarm on %s",alarmnames[LTEMP],ctime(&cur->atime));
            cur = cur->next;
        }
        else if (cur->code == HHUMID)
        {
            fprintf(stdout,"%s Alarm on %s",alarmnames[HHUMID],ctime(&cur->atime));
            cur = cur->next;
        }
        else if (cur->code == LHUMID)
        {
            fprintf(stdout,"%s Alarm on %s",alarmnames[LHUMID],ctime(&cur->atime));
            cur = cur->next;
        }
        else if (cur->code == HPRESS)
        {
            fprintf(stdout,"%s Alarm on %s",alarmnames[HPRESS],ctime(&cur->atime));
            cur = cur->next;
        }
        else if (cur->code == LPRESS)
        {
            fprintf(stdout,"%s Alarm on %s",alarmnames[LPRESS],ctime(&cur[NALARMS].atime));
            cur = cur->next;
        }
    }
}

/** @brief Sets One Alarm
 *  @version 13APRIL2021
 *  @author Jakob Wood
 *  @param object of alarm_e type
 *  @param object of time_t type
 *  @param double value
 *  @param pointer to alarm_s type
 *  @return int
*/
int GhSetOneAlarm(alarm_e code, time_t atime, double value, alarm_s * head)
{
    alarm_s *cur;
    alarm_s *last;
    cur = head;
    if (cur->code != NOALARM)
    {
        while (cur != NULL)
        {
            if(cur->code == code)
            {
                cur->atime = atime;
                cur->value = value;
                return 0;
            }
            last = cur;
            cur = cur->next;
        }
        cur = (alarm_s *) calloc(1,sizeof(alarm_s));
        if (cur == NULL)
        {
            return 0;
        }
        last->next = cur;
    }
    cur->code = code;
    cur->atime = atime;
    cur->value = value;
    cur->next = NULL;
    return 1;
}

/** @brief Clears Alarms
 *  @version 13APRIL2021
 *  @author Jakob Wood
 *  @param object of alarm_e type
 *  @param pointer to alarm_s type
 *  @return pointer to alarm_s type
*/
alarm_s * GhClearOneAlarm(alarm_e code, alarm_s * head)
{
    alarm_s * cur;
    alarm_s * last;
    cur = head;
    last = head;
    if ((cur->code == code) && (cur->next == NULL))
    {
        cur->code = NOALARM;
        return head;
    }
    if ((cur->code == code) && (cur->next != NULL))
    {
        head = cur->next;
        free(cur);
        return head;
    }
    while (cur != NULL)
    {
        if (cur->code == code)
        {
            last->next = cur->next;
            free(cur);
            return head;
        }
        last = cur;
        cur = cur->next;
    }
    return head;
}
