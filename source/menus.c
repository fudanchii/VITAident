#include <psp2/ctrl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <vita2d.h>

#include "misc.h"
#include "power.h"
#include "storage.h"
#include "system.h"
#include "utils.h"
#include "menus.h"
#include "arm/cpuinfo.h"

#define BACKGROUND_COLOUR      RGBA8(245, 245, 247, 255)
#define STATUS_BAR_COLOUR      RGBA8(42, 40, 41, 255)
#define MENU_BAR_COLOUR        RGBA8(255, 255, 255, 255)
#define ITEM_COLOUR            RGBA8(0, 0, 0, 255)
#define ITEM_SELECTED_COLOUR   MENU_BAR_COLOUR
#define MENU_SELECTOR_COLOUR   RGBA8(100, 141, 229, 255)
#define MENU_INFO_TITLE_COLOUR RGBA8(144, 137, 129, 255)
#define MENU_INFO_DESC_COLOUR  RGBA8(51, 51, 51, 255)
#define GAUGE_TOTAL_COLOUR     MENU_INFO_TITLE_COLOUR
#define GAUGE_FILL_NORM_COLOUR RGBA8(  0, 175, 185, 255)
#define GAUGE_FILL_CRIT_COLOUR RGBA8(240, 113, 103, 255)

#define MENU_Y_DIST    50
#define MAX_MENU_ITEMS 5

SceUInt32 pressed = 0;

static void Menu_DrawText(vita2d_font *font, int x, int y, char *title, const char *text, ...) {
    int title_width = 0;
    title_width = vita2d_font_text_width(font, 25, title);
    vita2d_font_draw_text(font, x, y, MENU_INFO_TITLE_COLOUR, 25, title);
    
    char buffer[128];
    va_list args;
    va_start(args, text);
    vsnprintf(buffer, 128, text, args);
    vita2d_font_draw_text(font, x + title_width + 10, y, MENU_INFO_DESC_COLOUR, 25, buffer);
    va_end(args);
}

static void Menu_SystemInfo(vita2d_font *font) {
    char real_version[8], spoofed_version[8], factory_version[8];
    char *model = NULL, *unit = NULL, *CID = NULL, *PSID = NULL;

    System_GetSystemSwVer(real_version);
    System_GetSystemSwVer2(spoofed_version);
    System_GetSystemSwVerFactory(factory_version);
    System_GetProduct(&model);
    System_GetUnit(&unit);
    System_GetConsoleID(&CID);
    System_GetPSID(&PSID);

    Menu_DrawText(font, 330, 235, "System software version:", "%s", real_version);
    Menu_DrawText(font, 330, 270, "Spoofed software version:", "%s", spoofed_version);
    Menu_DrawText(font, 330, 305, "Factory software version:", "%s", factory_version);
    Menu_DrawText(font, 330, 340, "Model:", model);
    Menu_DrawText(font, 330, 375, "Unit:", unit);
    Menu_DrawText(font, 330, 410, "Console ID:", CID);
    Menu_DrawText(font, 330, 445, "PS ID:", PSID);
}

static void Menu_BatteryInfo(vita2d_font *font) {
    char *batt_stat = NULL, *cable_stat = NULL;
    int percent = 0, SOH = 0, count = 0, full_capacity = 0, remain_capacity = 0, temp = 0, voltage = 0;
    
    Power_GetBatteryStatus(&batt_stat);
    Power_GetBatteryPercentage(&percent);
    Power_GetBatterySOH(&SOH);
    Power_GetBatteryCycleCount(&count);
    Power_GetBatteryFullCapacity(&full_capacity);
    Power_GetBatteryRemainCapacity(&remain_capacity);
    Power_GetBatteryTemp(&temp);
    Power_GetBatteryVoltage(&voltage);
    Power_GetCableStatus(&cable_stat);

    Menu_DrawText(font, 330, 235, "Charging:", "%s (cable %s)", batt_stat, cable_stat);
    Menu_DrawText(font, 330, 270, "Percentage:", "%d%%", percent);
    Menu_DrawText(font, 330, 305, "Cycle count:", "%d (SOH: %d%%)", count, SOH);
    Menu_DrawText(font, 330, 340, "Remaining/Full Capacity:", "%i mAh/%i mAh", remain_capacity, full_capacity);
    Menu_DrawText(font, 330, 375, "Temperature:", "%0.1f °C (%0.1f °F)", (temp / 100.0), ((1.8 * temp) / 100.0) + 32.0);
    Menu_DrawText(font, 330, 410, "Voltage:", "%0.1f V" , (voltage / 1000.0));
    Menu_DrawText(font, 330, 445, "CPU clock:", "%d MHz", Power_GetClockFrequency(ClockFrequencyTypeCPU));
    Menu_DrawText(font, 330, 480, "BUS clock:", "%d MHz", Power_GetClockFrequency(ClockFrequencyTypeBUS));
    Menu_DrawText(font, 330, 515, "GPU clock:",
        Power_GetClockFrequency(ClockFrequencyTypeGPUXbar) > 0 ?
                                                "%d MHz (Xbar: %d MHz)" :
                                                "%d MHz", Power_GetClockFrequency(ClockFrequencyTypeGPU),
                                                          Power_GetClockFrequency(ClockFrequencyTypeGPUXbar));

}

static void  Menu_ProcessorsInfo(vita2d_font *font) {
    char cpuVendor[48];
    char cpuFamily[15];
    int var, rev, mpSupport, cores;

    cpuinfo_getVendorString(CpuMainId, &cpuVendor[0], 48);
    cpuinfo_getCpuFamily(CpuMainId, &cpuFamily[0], 15);
    var = cpuinfo_getVariant(CpuMainId);
    rev = cpuinfo_getRevision(CpuMainId);

    mpSupport = cpuinfo_getMPSupport(CpuMpId);
    cores = CpuCount;

    Menu_DrawText(font, 330, 235, "Implementor:",  "%s", &cpuVendor[0]);
    Menu_DrawText(font, 330, 270, "Series:",       "%s %sr%dp%d",
        &cpuFamily[0],
        mpSupport ? "MPCore " : "",
        var,
        rev);
    Menu_DrawText(font, 330, 305, "# of Cores:",   "%d", cores);
}

static void Menu_StorageInfo(vita2d_font *font, vita2d_texture *texture) {
    char *devices[] = {
        "ur0:",
        "ux0:",
        "imc0:",
        "uma0:",
        "xmc0:"
    };
    
    SceOff sizes_total[5], sizes_free[5], sizes_used[5];
    int ret = 0, devices_found = 0;
    static char free_str[16], total_str[16];
    int max_gauge = 480;
    
    for (int i = 0; i < 5; i++) {
        if ((R_SUCCEEDED(ret = Storage_GetTotalCapacity(devices[i], &sizes_total[i]))) && 
            (R_SUCCEEDED(ret = Storage_GetFreeCapacity(devices[i], &sizes_free[i]))) && 
            (R_SUCCEEDED(ret = Storage_GetUsedCapacity(devices[i], &sizes_used[i])))) {

            memset(free_str, 0, 16);
            memset(total_str, 0, 16);

            Storage_GetSizeString(free_str, sizes_free[i]);
            Storage_GetSizeString(total_str, sizes_total[i]);

            vita2d_draw_texture(texture, 320, 215 + (86 * devices_found));
            Menu_DrawText(font, 400, 235 + (86 * devices_found), devices[i], "%s / %s", free_str, total_str);

            SceOff gauge = (SceOff)(((double)sizes_used[i] / (double)sizes_total[i]) * (double)max_gauge);
            vita2d_draw_rectangle(400, 260 + (86 * devices_found), max_gauge, 20, GAUGE_TOTAL_COLOUR);
            vita2d_draw_rectangle(400, 260 + (86 * devices_found), gauge,     20,
                    gauge >= (SceOff)(((double)88 / (double)100) * (double)max_gauge) ?
                        GAUGE_FILL_CRIT_COLOUR :
                        GAUGE_FILL_NORM_COLOUR);

            devices_found++;
        }
    }
}

static void Menu_NetInfo(vita2d_font *font) {
    char mac[20], ip[20];
    unsigned int percent = 0;

    Misc_GetMacAddress(mac);
    Misc_GetIPAddress(ip);
    Misc_GetRSSIPercent(&percent);

    Menu_DrawText(font, 330, 235, "MAC address:", mac);
    Menu_DrawText(font, 330, 270, "IP address:", ip);
    Menu_DrawText(font, 330, 305, "RSSI signal strength:", "%d%%", percent);
}

static void Menu_SetMax(int *set, int value, int max) {
	if (*set > max)
		*set = value;
}

static void Menu_SetMin(int *set, int value, int min) {
	if (*set < min)
		*set = value;
}

void Menu_Main(void) {
    vita2d_texture *banner = NULL, *drive_icon = NULL;
    vita2d_font *font = NULL;

    font = vita2d_load_font_file("app0:res/Ubuntu-R.ttf");

    banner = vita2d_load_PNG_file("app0:sce_sys/icon0.png");
    vita2d_texture_set_filters(banner, SCE_GXM_TEXTURE_FILTER_LINEAR, SCE_GXM_TEXTURE_FILTER_LINEAR);

    drive_icon = vita2d_load_PNG_file("app0:res/drive.png");
    vita2d_texture_set_filters(drive_icon, SCE_GXM_TEXTURE_FILTER_LINEAR, SCE_GXM_TEXTURE_FILTER_LINEAR);

    vita2d_set_clear_color(BACKGROUND_COLOUR);

    int font_height = 0, selection = 0, SCE_CTRL_ENTER = Utils_GetEnterButton();
    font_height = vita2d_font_text_height(font, 25, "VITAident");

    const char *menu_item[] = {
        "System",
        "Processors",
        "Battery",
        "Storage",
        "WiFi",
        "Exit"
    };

    while(SCE_TRUE) {
        vita2d_start_drawing();
        vita2d_clear_screen();
        
        vita2d_draw_rectangle(0, 0, 960, 38, STATUS_BAR_COLOUR);
        vita2d_draw_rectangle(0, 38, 300, 506, MENU_BAR_COLOUR);

        vita2d_font_draw_text(font, 20, ((38 - font_height) / 2) + 20, BACKGROUND_COLOUR, 25, "VITAident v2.0B");

        vita2d_draw_rectangle(0, 38 + (MENU_Y_DIST * selection), 300, MENU_Y_DIST, MENU_SELECTOR_COLOUR);

        if (selection != MAX_MENU_ITEMS) {
            vita2d_draw_texture(banner, 566, 60);
        }

        for (int i = 0; i < MAX_MENU_ITEMS + 1; i++) {
            vita2d_font_draw_text(font, 30, 70 + MENU_Y_DIST * i, selection == i? ITEM_SELECTED_COLOUR : ITEM_COLOUR, 25, menu_item[i]);
        }

        switch(selection) {
            case 0:
                Menu_SystemInfo(font);
                break;

            case 1:
                Menu_ProcessorsInfo(font);
                break;

            case 2:
                Menu_BatteryInfo(font);
                break;

            case 3:
                Menu_StorageInfo(font, drive_icon);
                break;

            case 4:
                Menu_NetInfo(font);
                break;
            
            default:
                break;
        }

        vita2d_end_drawing();
        vita2d_swap_buffers();

        Utils_ReadControls();
        
        if (pressed & SCE_CTRL_UP)
            selection--;
        else if (pressed & SCE_CTRL_DOWN)
            selection++;
            
        Menu_SetMax(&selection, 0, MAX_MENU_ITEMS);
        Menu_SetMin(&selection, MAX_MENU_ITEMS, 0);
        
        if ((pressed & SCE_CTRL_START) || ((pressed & SCE_CTRL_ENTER) && (selection == MAX_MENU_ITEMS)))
            break;
    }

    vita2d_wait_rendering_done();
    vita2d_free_texture(drive_icon);
    vita2d_free_texture(banner);
    vita2d_free_font(font);
}
