#include "python_playground.h"

// ##############
// TICK
void tick_callback(void* ctx) {
    FURI_LOG_D(LOG_TAG, "Tick");
    PPApp_t* app = ctx;
    if (app->cli_running) {
        cli_write(app->cli, (uint8_t*)"Tick", 4);
        cli_write(app->cli, (uint8_t*)"\r\n", 2);
        return;
    }
}

// ##############
// CLI
void cli_callback(Cli* cli, FuriString* args, void* ctx) {
    UNUSED(args);
    FURI_LOG_I(LOG_TAG, "CLI connection...");

    PPApp_t* app = ctx;
    if (app->cli_running) {
        return;
    }

    app->cli = cli;
    app->cli_running = true;
    cli_write(app->cli, (uint8_t*)("CLI Connection Start"), 20);
    cli_write(app->cli, (uint8_t*)"\r\n", 2);
    view_dispatcher_switch_to_view(app->view_dispatcher, CONNECTED_VIEW_ID);    
    while (app->cli_running) {
        if (!cli_is_connected(app->cli)) {
            app->cli_running = false;
            break;
        }
        // recieve payload
        ProtocolData_t* data = protocol_recieve(app->cli, 5);
        if (data == NULL) {
            continue;
        }
        GuiDrawStrData_t* draw_str_data = data->data;
        ConnectedViewModel_t* vm = view_get_model(app->connected_view);
        free(vm->draw_data);
        vm->draw_data = draw_str_data;
        view_commit_model(app->connected_view, true);
    }
    // const char* close = "CLOSE";
    // ProtocolPayload_t* payload = protocol_payload_alloc(TEST_ID, (uint8_t*)close, strlen(close));
    // protocol_payload_send(payload, app->cli);

    app->cli = NULL;
    view_dispatcher_switch_to_view(app->view_dispatcher, DISCONNECTED_VIEW_ID);
}

// ##############
// STATE
PPApp_t* state_init() {
    FURI_LOG_I(LOG_TAG, "Initing app...");
    PPApp_t* app = malloc(sizeof(PPApp_t));

    app->view_dispatcher    = view_dispatcher_alloc();
    app->gui                = furi_record_open(RECORD_GUI);
    app->mutex              = furi_mutex_alloc(FuriMutexTypeNormal);
    app->cli                = NULL;

    app->cli_running = false;

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_enable_queue(app->view_dispatcher);
    //view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    //view_dispatcher_set_tick_event_callback(app->view_dispatcher, tick_callback, 1000);

    app->connected_view = connected_view_alloc(app);
    app->disconnected_view = disconnected_view_alloc(app);

    return app;
}

void state_free(PPApp_t* app) {
    FURI_LOG_I(LOG_TAG, "Freeing app...");
    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_GUI);
    view_free(app->connected_view);
    view_free(app->disconnected_view);
    furi_mutex_free(app->mutex);
    free(app);
}

// ##############
// ENTRY
int32_t python_playground_app(void) {
    PPApp_t* app = state_init();
    
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_add_command(cli, "python_playground", CliCommandFlagParallelSafe, cli_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, DISCONNECTED_VIEW_ID);
    view_dispatcher_run(app->view_dispatcher);

    cli_delete_command(cli, "python_playground");
    furi_record_close(RECORD_CLI);

    state_free(app);

    return 0;
}
