#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <term.h>
#include <ncurses.h>
#include <libgen.h>
#include <signal.h>
#include <stdlib.h>
#include <fstream>


#include "cmd_process.h"
#include "PS3Controller.h"
#include "JointData.h"

#define MOTION_FILE_PATH    "../../../Data/motion_4096.bin"

#define INI_FILE_PATH       "../../../Data/config.ini"

#define KINECT_FILE_PATH    "kinect_joints.txt"

using namespace Robot;
using namespace std;

LinuxCM730 linux_cm730("/dev/ttyUSB0");
CM730 cm730(&linux_cm730);
LinuxMotionTimer linuxMotionTimer;

void change_current_dir()
{
    char exepath[1024] = {0};
    if(readlink("/proc/self/exe", exepath, sizeof(exepath)) != -1)
        chdir(dirname(exepath));
}

void sighandler(int sig)
{
    struct termios term;
    tcgetattr( STDIN_FILENO, &term );
    term.c_lflag |= ICANON | ECHO;
		tcsetattr( STDIN_FILENO, TCSANOW, &term );

    exit(0);
}

int main(int argc, char *argv[])
{
    signal(SIGABRT, &sighandler);
    signal(SIGTERM, &sighandler);
    signal(SIGQUIT, &sighandler);
    signal(SIGINT, &sighandler);

    int ch;
    char filename[128];

    minIni* ini = new minIni(INI_FILE_PATH);

    change_current_dir();
    if(argc < 2)
        strcpy(filename, MOTION_FILE_PATH); // Set default motion file path
    else
        strcpy(filename, argv[1]);

    /////////////// Load/Create Action File //////////////////
    if(Action::GetInstance()->LoadFile(filename) == false)
    {
        printf("Can not open %s\n", filename);
        printf("Do you want to make a new action file? (y/n) ");
        ch = _getch();
        if(ch != 'y')
        {
            printf("\n");
            exit(0);
        }

        if(Action::GetInstance()->CreateFile(filename) == false)
        {
            printf("Fail to create %s\n", filename);
            exit(0);
        }
    }

    if(MotionManager::GetInstance()->Initialize(&cm730) == false)
    {
        printf("Initializing Motion Manager failed!\n");
        exit(0);
    }

    MotionManager::GetInstance()->LoadINISettings(ini);
    MotionManager::GetInstance()->SetEnable(false);
    MotionManager::GetInstance()->AddModule((MotionModule*)Action::GetInstance());
    linuxMotionTimer.Initialize(MotionManager::GetInstance());
    linuxMotionTimer.Stop();

    ifstream joints_file;
    int angles[4];
    joints_file.open(KINECT_FILE_PATH);

    if (!joints_file.is_open())
    {
        printf("Could not open Kinect joints file.\n");
        exit(0);
    }

    int i = 0;
    char angle_buf[4];  // Angles should never be longer than 3 characters (min: 0, max: 300) + '\0'

    while (joints_file.good())
    {
        joints_file.getline(angle_buf, 4, ',');
        angles[i] = atoi(angle_buf);
        ++i;
    }

    joints_file.close();

    // Joint data had 150 added to it before being sent by Kinect so that no
    // negative numbers needed to be parsed. Must return back to the actual angle
    // scale of -150 to 150
    int r_shoulder_pitch = angles[0] - 150;
    int r_shoulder_roll = angles[1] - 150;
    int l_shoulder_pitch = angles[2] - 150;
    int l_shoulder_roll = angles[3] - 150;

    cout << "r_shoulder_pitch: " << r_shoulder_pitch << endl;
    cout << "r_shoulder_roll: " << r_shoulder_roll << endl;
    cout << "l_shoulder_pitch: " << l_shoulder_pitch << endl;
    cout << "l_shoulder_pitch: " << l_shoulder_roll << endl;

    MotionStatus::m_CurrentJoints.SetEnableUpperBodyWithoutHead(true, true);
    MotionStatus::m_CurrentJoints.SetValue(MotionStatus::m_CurrentJoints.ID_R_SHOULDER_PITCH, r_shoulder_pitch);
    MotionStatus::m_CurrentJoints.SetValue(MotionStatus::m_CurrentJoints.ID_R_SHOULDER_ROLL, r_shoulder_roll);
    MotionStatus::m_CurrentJoints.SetValue(MotionStatus::m_CurrentJoints.ID_L_SHOULDER_PITCH, l_shoulder_pitch);
    MotionStatus::m_CurrentJoints.SetValue(MotionStatus::m_CurrentJoints.ID_L_SHOULDER_ROLL, l_shoulder_roll);

    exit(0);
}
