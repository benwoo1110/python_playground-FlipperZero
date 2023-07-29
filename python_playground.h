#include <stdlib.h>
#include <stdio.h>

#include <furi.h>
#include <cli/cli.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <gui/view_dispatcher.h>
#include <gui/view.h>
#include <input/input.h>

#include "protocol.h"

#define LOG_TAG "PyPg"
#define CLI_NAME "python_playground"

#define CONNECTED_VIEW_ID 1
#define DISCONNECTED_VIEW_ID 2

typedef struct ConnectedViewModel {
    GuiDrawData_t*   draw_data;
    char*            debug_msg;
} ConnectedViewModel_t;

typedef struct Payload {
    uint8_t*    data;
    uint32_t    size;
} Payload_t;

typedef struct PPApp {
    // Flipper Firmware Variables 
    ViewDispatcher*     view_dispatcher;
    Gui*                gui;
    FuriMutex**         mutex;
    Cli*                cli;
    
    // App Variables
    bool    cli_running;

    // Views
    View*   connected_view;
    View*   disconnected_view;
} PPApp_t;

View* connected_view_alloc(PPApp_t* app);

View* disconnected_view_alloc(PPApp_t* app);
