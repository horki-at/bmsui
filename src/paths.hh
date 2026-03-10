#ifndef INCLUDED_SCRIPTS_
#define INCLUDED_SCRIPTS_

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define START_COMM_SCRIPT (char *)(TOSTRING(SCRDIR) "/util/start_comm.sh")
#define PYTHON3_BIN (char *)(TOSTRING(SCRDIR) "/.venv/bin/python3")
#define POPULATE_BMS_SCRIPT (char *)(TOSTRING(SCRDIR) "/util/populate_bms.py")
#define FONT_PATH (TOSTRING(SCRDIR) "/assets/iosevka-regular.ttf")

#define TTYVBMS (TOSTRING(TMPDIR) "/ttyVBMS")
#define TTYVAPP (TOSTRING(TMPDIR) "/ttyVAPP")

#define IMGUI_INI (char *)(TOSTRING(SCRDIR) "/assets/imgui.ini")

#endif // INCLUDED_SCRIPTS_
