#include <stdlib.h>
#include <stdio.h>

#include <m-dict.h>
#include <furi.h>
#include <furi_hal.h>
#include <cli/cli.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <gui/icon.h>
#include <gui/icon_i.h>
#include <gui/view_dispatcher.h>
#include <gui/view.h>
#include <input/input.h>

#include "protocol.h"

#define LOG_TAG "PyPg"
#define CLI_NAME "python_playground"

#define CONNECTED_VIEW_ID 1
#define DISCONNECTED_VIEW_ID 2

DICT_DEF2(IconDict, uint8_t, M_DEFAULT_OPLIST, Icon*, M_PTR_OPLIST)

typedef struct ConnectedViewModel {
    GuiDrawData_t*   draw_data;
    char*            debug_msg;
    IconDict_t       icons;
} ConnectedViewModel_t;

typedef struct Payload {
    uint8_t*    data;
    uint32_t    size;
} Payload_t;

typedef struct PPApp {
    // Flipper Firmware Variables 
    ViewDispatcher* view_dispatcher;
    Gui*            gui;
    FuriMutex**     mutex;
    Cli*            cli;
    
    // App Variables
    bool            cli_running;

    // Views
    View*           connected_view;
    View*           disconnected_view;
} PPApp_t;

View* connected_view_alloc(PPApp_t* app);

View* disconnected_view_alloc(PPApp_t* app);
